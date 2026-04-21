#include "loader.hpp"
#include <stdexcept>
#include <string>
#include <vector>

Loader::Loader(const std::string& db_path) {
    if (sqlite3_open(db_path.c_str(), &db_) != SQLITE_OK) {
        throw_sqlite_error("Failed to open database");
    }
}

Loader::~Loader() {
    finalize_statements();
    sqlite3_close(db_);
}

void Loader::init_schema() {
    const char* create_cards_table = R"sql(
        CREATE TABLE IF NOT EXISTS oracle_cards (
            o_id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            mana_cost TEXT NOT NULL,
            cmc INTEGER NOT NULL,
            type_line_raw TEXT NOT NULL,
            oracle_text TEXT NOT NULL,
            type_encoded INTEGER NOT NULL,
            color INTEGER NOT NULL,
            color_identity INTEGER NOT NULL,
            legality_exceptions INTEGER NOT NULL,
            legality_baseline INTEGER NOT NULL,
            keywords_str TEXT NOT NULL,
            power INTEGER,
            toughness INTEGER,
            cda_flags INTEGER,
            loyalty INTEGER,
            defense INTEGER
        );
    )sql";

    const char* create_prints_table = R"sql(
        CREATE TABLE IF NOT EXISTS printed_cards (
            id TEXT PRIMARY KEY,
            o_id TEXT NOT NULL,
            set TEXT NOT NULL,
            set_id TEXT NOT NULL,
            set_name TEXT NOT NULL,
            set_type TEXT NOT NULL,
            collector_number TEXT NOT NULL,
            artist TEXT NOT NULL,
            border_color TEXT NOT NULL,
            frame TEXT NOT NULL,
            released_at TEXT NOT NULL,
            image_status TEXT NOT NULL,
            rarity INTEGER NOT NULL,
            layout INTEGER NOT NULL,
            flags INTEGER NOT NULL,
            usd_price REAL,
            usd_foil_price REAL,
            multiverse_ids_str TEXT,
            games_str TEXT,
            finishes_str TEXT,
            scryfall_uri TEXT,
            rulings_uri TEXT,
            prints_search_uri TEXT
        );
    )sql";

    if (sqlite3_exec(db_, create_cards_table, nullptr, nullptr, nullptr) != SQLITE_OK ||
        sqlite3_exec(db_, create_prints_table, nullptr, nullptr, nullptr) != SQLITE_OK) {
        throw_sqlite_error("Failed to create table");
    }
}

void Loader::prepare_statements() {
    const char* insert_card_query = R"sql(
        INSERT OR REPLACE INTO oracle_cards (
            o_id, name, mana_cost, cmc, type_line_raw, oracle_text,
            type_encoded, color, color_identity, legality_exceptions,
            legality_baseline, keywords_str, power, toughness, cda_flags,
            loyalty, defense
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )sql";

    const char* insert_print_query = R"sql(
        INSERT OR REPLACE INTO printed_cards (
            id, o_id, set, set_id, set_name, set_type, collector_number,
            artist, border_color, frame, released_at, image_status, rarity,
            layout, flags, usd_price, usd_foil_price, multiverse_ids_str,
            games_str, finishes_str, scryfall_uri, rulings_uri, prints_search_uri
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )sql";

    const char* query_card_query = R"sql(
        SELECT * FROM oracle_cards WHERE o_id = ?;
    )sql";

    const char* query_print_query = R"sql(
        SELECT * FROM printed_cards WHERE id = ?;
    )sql";

    if (sqlite3_prepare_v2(db_, insert_card_query, -1, &stmt_card_, nullptr) != SQLITE_OK ||
        sqlite3_prepare_v2(db_, insert_print_query, -1, &stmt_print_, nullptr) != SQLITE_OK ||
        sqlite3_prepare_v2(db_, query_card_query, -1, &stmt_query_card_, nullptr) != SQLITE_OK ||
        sqlite3_prepare_v2(db_, query_print_query, -1, &stmt_query_print_, nullptr) != SQLITE_OK) {
        throw_sqlite_error("Failed to prepare statement");
    }
}

void Loader::finalize_statements() {
    if (stmt_card_) sqlite3_finalize(stmt_card_);
    if (stmt_print_) sqlite3_finalize(stmt_print_);
    if (stmt_query_card_) sqlite3_finalize(stmt_query_card_);
    if (stmt_query_print_) sqlite3_finalize(stmt_query_print_);
}

void Loader::begin_transaction() {
    if (sqlite3_exec(db_, "BEGIN;", nullptr, nullptr, nullptr) != SQLITE_OK) {
        throw_sqlite_error("Failed to begin transaction");
    }
}

void Loader::commit_transaction() {
    if (sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, nullptr) != SQLITE_OK) {
        throw_sqlite_error("Failed to commit transaction");
    }
}

void Loader::rollback_transaction() {
    if (sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr) != SQLITE_OK) {
        throw_sqlite_error("Failed to rollback transaction");
    }
}

void Loader::bind_and_step_card(const TransformedCard& card) {
    // Bind values for the card
    sqlite3_reset(stmt_card_);
    sqlite3_bind_text(stmt_card_, 1, card.o_id.c_str(), -1, SQLITE_STATIC);
    // ... bind other fields similarly ...
    if (sqlite3_step(stmt_card_) != SQLITE_DONE) {
        throw_sqlite_error("Failed to insert or replace card");
    }
}

void Loader::bind_and_step_print(const TransformedPrint& print) {
    // Bind values for the print
    sqlite3_reset(stmt_print_);
    sqlite3_bind_text(stmt_print_, 1, print.id.c_str(), -1, SQLITE_STATIC);
    // ... bind other fields similarly ...
    if (sqlite3_step(stmt_print_) != SQLITE_DONE) {
        throw_sqlite_error("Failed to insert or replace print");
    }
}

void Loader::load_card(const TransformedCard& card) {
    bind_and_step_card(card);
}

void Loader::load_print(const TransformedPrint& print) {
    bind_and_step_print(print);
}

void Loader::load_cards(const std::vector<TransformedCard>& cards) {
    begin_transaction();
    try {
        for (const auto& card : cards) {
            load_card(card);
        }
        commit_transaction();
    } catch (...) {
        rollback_transaction();
        throw;
    }
}

void Loader::load_prints(const std::vector<TransformedPrint>& prints) {
    begin_transaction();
    try {
        for (const auto& print : prints) {
            load_print(print);
        }
        commit_transaction();
    } catch (...) {
        rollback_transaction();
        throw;
    }
}

std::optional<TransformedCard> Loader::get_card_by_o_id(const std::string& o_id) const {
    sqlite3_reset(stmt_query_card_);
    sqlite3_bind_text(stmt_query_card_, 1, o_id.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt_query_card_) == SQLITE_ROW) {
        TransformedCard card;
        // Retrieve values from the result set
        // ... populate card fields ...
        return card;
    }

    return std::nullopt;
}

std::optional<TransformedPrint> Loader::get_print_by_id(const std::string& id) const {
    sqlite3_reset(stmt_query_print_);
    sqlite3_bind_text(stmt_query_print_, 1, id.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt_query_print_) == SQLITE_ROW) {
        TransformedPrint print;
        // Retrieve values from the result set
        // ... populate print fields ...
        return print;
    }

    return std::nullopt;
}

[[noreturn]] void Loader::throw_sqlite_error(const std::string& context) const {
    throw LoaderError(context + ": " + sqlite3_errmsg(db_));
}