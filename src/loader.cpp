#include "../include/loader.hpp"
#include <fmt/core.h>

// ============================================================
//  DDL
// ============================================================

// oracle_cards: one row per unique oracle_id.
//
// type_encoded  → TypeEncoding::raw()  (uint64, stored as INTEGER)
// color / color_identity → ColorEncoding::raw() (uint16, stored as INTEGER)
// legality_exceptions    → LegalityEncoding::exceptions (uint32)
// legality_baseline      → LegalityEncoding::baseline   (0 or 1)
// power / toughness      → signed (NegativePT cards have negative power)
// cda_flags              → NULL for BasePT/NegativePT, 1/2/3 for CDAPT
static const char* DDL_ORACLE_CARDS = R"sql(
CREATE TABLE IF NOT EXISTS oracle_cards (
    o_id                 TEXT    PRIMARY KEY,
    name                 TEXT    NOT NULL,
    mana_cost            TEXT    NOT NULL DEFAULT '',
    cmc                  INTEGER NOT NULL DEFAULT 0,
    type_line_raw        TEXT    NOT NULL,
    type_encoded         INTEGER NOT NULL DEFAULT 0,
    oracle_text          TEXT    NOT NULL DEFAULT '',
    color                INTEGER NOT NULL DEFAULT 0,
    color_identity       INTEGER NOT NULL DEFAULT 0,
    legality_exceptions  INTEGER NOT NULL DEFAULT 0,
    legality_baseline    INTEGER NOT NULL DEFAULT 0,
    keywords             TEXT    NOT NULL DEFAULT '',
    power                INTEGER,
    toughness            INTEGER,
    cda_flags            INTEGER,
    loyalty              INTEGER,
    defense              INTEGER
);
)sql";

// printed_cards: one row per unique Scryfall printing id.
//
// flags            → PrintFlagBit bitmask (uint16)
// multiverse_ids   → comma-delimited integer string, empty if absent
// games / finishes → pipe-delimited string, empty if absent
// usd_price / usd_foil_price → REAL, NULL if absent
// scryfall_uri / rulings_uri / prints_search_uri → empty string if absent
static const char* DDL_PRINTED_CARDS = R"sql(
CREATE TABLE IF NOT EXISTS printed_cards (
    id                   TEXT    PRIMARY KEY,
    o_id                 TEXT    NOT NULL REFERENCES oracle_cards(o_id),
    set_code             TEXT    NOT NULL DEFAULT '',
    set_id               TEXT    NOT NULL DEFAULT '',
    set_name             TEXT    NOT NULL DEFAULT '',
    set_type             TEXT    NOT NULL DEFAULT '',
    collector_number     TEXT    NOT NULL DEFAULT '',
    artist               TEXT    NOT NULL DEFAULT '',
    border_color         TEXT    NOT NULL DEFAULT '',
    frame                TEXT    NOT NULL DEFAULT '',
    released_at          TEXT    NOT NULL DEFAULT '',
    image_status         TEXT    NOT NULL DEFAULT '',
    rarity               TEXT    NOT NULL DEFAULT '',
    layout               INTEGER NOT NULL DEFAULT 0,
    flags                INTEGER NOT NULL DEFAULT 0,
    usd_price            REAL,
    usd_foil_price       REAL,
    multiverse_ids       TEXT    NOT NULL DEFAULT '',
    games                TEXT    NOT NULL DEFAULT '',
    finishes             TEXT    NOT NULL DEFAULT '',
    scryfall_uri         TEXT    NOT NULL DEFAULT '',
    rulings_uri          TEXT    NOT NULL DEFAULT '',
    prints_search_uri    TEXT    NOT NULL DEFAULT ''
);
)sql";

static const char* DDL_IDX_PRINT_ORACLE = R"sql(
CREATE INDEX IF NOT EXISTS idx_printed_cards_o_id
    ON printed_cards(o_id);
)sql";

static const char* DDL_IDX_ORACLE_NAME = R"sql(
CREATE INDEX IF NOT EXISTS idx_oracle_cards_name
    ON oracle_cards(name);
)sql";

// ============================================================
//  Prepared statement SQL
//  INSERT OR REPLACE → idempotent re-runs against fresh bulk data.
// ============================================================

static const char* SQL_UPSERT_CARD = R"sql(
INSERT OR REPLACE INTO oracle_cards (
    o_id, name, mana_cost, cmc,
    type_line_raw, type_encoded, oracle_text,
    color, color_identity,
    legality_exceptions, legality_baseline,
    keywords,
    power, toughness, cda_flags, loyalty, defense
) VALUES (
    ?, ?, ?, ?,
    ?, ?, ?,
    ?, ?,
    ?, ?,
    ?,
    ?, ?, ?, ?, ?
);
)sql";

static const char* SQL_UPSERT_PRINT = R"sql(
INSERT OR REPLACE INTO printed_cards (
    id, o_id,
    set_code, set_id, set_name, set_type,
    collector_number, artist, border_color, frame,
    released_at, image_status,
    rarity, layout, flags,
    usd_price, usd_foil_price,
    multiverse_ids, games, finishes,
    scryfall_uri, rulings_uri, prints_search_uri
) VALUES (
    ?, ?,
    ?, ?, ?, ?,
    ?, ?, ?, ?,
    ?, ?,
    ?, ?, ?,
    ?, ?,
    ?, ?, ?,
    ?, ?, ?
);
)sql";

// ============================================================
//  Constructor / Destructor
// ============================================================

Loader::Loader(const std::string& db_path) {
    int rc = sqlite3_open(db_path.c_str(), &db_);
    if (rc != SQLITE_OK)
        throw LoaderError(fmt::format("sqlite3_open('{}') failed: {}",
                                      db_path, sqlite3_errmsg(db_)));

    // WAL for better concurrent read performance during analytics queries
    sqlite3_exec(db_, "PRAGMA journal_mode=WAL;",   nullptr, nullptr, nullptr);
    // NORMAL is safe for a local analytics warehouse and much faster than FULL
    sqlite3_exec(db_, "PRAGMA synchronous=NORMAL;", nullptr, nullptr, nullptr);
    // Enforce FK constraint so prints can't be loaded before their oracle card
    sqlite3_exec(db_, "PRAGMA foreign_keys=ON;",    nullptr, nullptr, nullptr);
}

Loader::~Loader() {
    finalize_statements();
    if (db_) sqlite3_close(db_);
}

// ============================================================
//  Schema + statement lifecycle
// ============================================================

void Loader::init_schema() {
    char* errmsg = nullptr;
    auto exec = [&](const char* sql) {
        int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &errmsg);
        if (rc != SQLITE_OK) {
            std::string msg = errmsg ? errmsg : "unknown error";
            sqlite3_free(errmsg);
            throw LoaderError(fmt::format("init_schema failed: {}", msg));
        }
    };
    exec(DDL_ORACLE_CARDS);
    exec(DDL_PRINTED_CARDS);
    exec(DDL_IDX_PRINT_ORACLE);
    exec(DDL_IDX_ORACLE_NAME);
    prepare_statements();
}

void Loader::prepare_statements() {
    auto prep = [&](const char* sql, sqlite3_stmt** stmt) {
        int rc = sqlite3_prepare_v2(db_, sql, -1, stmt, nullptr);
        if (rc != SQLITE_OK)
            throw LoaderError(fmt::format("sqlite3_prepare_v2 failed: {}",
                                          sqlite3_errmsg(db_)));
    };
    prep(SQL_UPSERT_CARD,  &stmt_card_);
    prep(SQL_UPSERT_PRINT, &stmt_print_);
}

void Loader::finalize_statements() {
    if (stmt_card_)  { sqlite3_finalize(stmt_card_);  stmt_card_  = nullptr; }
    if (stmt_print_) { sqlite3_finalize(stmt_print_); stmt_print_ = nullptr; }
}

// ============================================================
//  Transaction helpers
// ============================================================

void Loader::begin_transaction() {
    if (sqlite3_exec(db_, "BEGIN;", nullptr, nullptr, nullptr) != SQLITE_OK)
        throw_sqlite_error("BEGIN transaction");
}

void Loader::commit_transaction() {
    if (sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, nullptr) != SQLITE_OK)
        throw_sqlite_error("COMMIT transaction");
}

void Loader::rollback_transaction() {
    sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);  // best-effort
}

// ============================================================
//  Bind helpers
// ============================================================

static void bind_text(sqlite3_stmt* stmt, int col, const std::string& s) {
    sqlite3_bind_text(stmt, col, s.c_str(), static_cast<int>(s.size()),
                      SQLITE_TRANSIENT);
}

template<typename T>
static void bind_optional_int(sqlite3_stmt* stmt, int col,
                               const std::optional<T>& val) {
    if (val.has_value())
        sqlite3_bind_int64(stmt, col,
                           static_cast<sqlite3_int64>(val.value()));
    else
        sqlite3_bind_null(stmt, col);
}

static void bind_optional_double(sqlite3_stmt* stmt, int col,
                                  const std::optional<double>& val) {
    if (val.has_value())
        sqlite3_bind_double(stmt, col, val.value());
    else
        sqlite3_bind_null(stmt, col);
}

// ============================================================
//  Bind + step implementations
// ============================================================

void Loader::bind_and_step_card(const TransformedCard& c) {
    sqlite3_reset(stmt_card_);
    sqlite3_clear_bindings(stmt_card_);

    bind_text          (stmt_card_,  1, c.o_id);
    bind_text          (stmt_card_,  2, c.name);
    bind_text          (stmt_card_,  3, c.mana_cost);
    sqlite3_bind_int   (stmt_card_,  4, static_cast<int>(c.cmc));
    bind_text          (stmt_card_,  5, c.type_line_raw);
    sqlite3_bind_int64 (stmt_card_,  6, static_cast<sqlite3_int64>(c.type_encoded));
    bind_text          (stmt_card_,  7, c.oracle_text);
    sqlite3_bind_int   (stmt_card_,  8, static_cast<int>(c.color));
    sqlite3_bind_int   (stmt_card_,  9, static_cast<int>(c.color_identity));
    sqlite3_bind_int64 (stmt_card_, 10, static_cast<sqlite3_int64>(c.legality_exceptions));
    sqlite3_bind_int   (stmt_card_, 11, static_cast<int>(c.legality_baseline));
    bind_text          (stmt_card_, 12, c.keywords_str);
    bind_optional_int  (stmt_card_, 13, c.power);
    bind_optional_int  (stmt_card_, 14, c.toughness);
    bind_optional_int  (stmt_card_, 15, c.cda_flags);
    bind_optional_int  (stmt_card_, 16, c.loyalty);
    bind_optional_int  (stmt_card_, 17, c.defense);

    if (sqlite3_step(stmt_card_) != SQLITE_DONE)
        throw_sqlite_error(fmt::format("load_card('{}')", c.o_id));
}

void Loader::bind_and_step_print(const TransformedPrint& p) {
    sqlite3_reset(stmt_print_);
    sqlite3_clear_bindings(stmt_print_);

    bind_text          (stmt_print_,  1, p.id);
    bind_text          (stmt_print_,  2, p.o_id);
    bind_text          (stmt_print_,  3, p.set);
    bind_text          (stmt_print_,  4, p.set_id);
    bind_text          (stmt_print_,  5, p.set_name);
    bind_text          (stmt_print_,  6, p.set_type);
    bind_text          (stmt_print_,  7, p.collector_number);
    bind_text          (stmt_print_,  8, p.artist);
    bind_text          (stmt_print_,  9, p.border_color);
    bind_text          (stmt_print_, 10, p.frame);
    bind_text          (stmt_print_, 11, p.released_at);
    bind_text          (stmt_print_, 12, p.image_status);
    // rarity is a single char — send as a 1-byte string
    std::string rarity_str(1, p.rarity);
    bind_text          (stmt_print_, 13, rarity_str);
    sqlite3_bind_int   (stmt_print_, 14, static_cast<int>(p.layout));
    sqlite3_bind_int   (stmt_print_, 15, static_cast<int>(p.flags));
    bind_optional_double(stmt_print_, 16, p.usd_price);
    bind_optional_double(stmt_print_, 17, p.usd_foil_price);
    bind_text          (stmt_print_, 18, p.multiverse_ids_str);
    bind_text          (stmt_print_, 19, p.games_str);
    bind_text          (stmt_print_, 20, p.finishes_str);
    bind_text          (stmt_print_, 21, p.scryfall_uri);
    bind_text          (stmt_print_, 22, p.rulings_uri);
    bind_text          (stmt_print_, 23, p.prints_search_uri);

    if (sqlite3_step(stmt_print_) != SQLITE_DONE)
        throw_sqlite_error(fmt::format("load_print('{}')", p.id));
}

// ============================================================
//  Public API
// ============================================================

void Loader::load_card(const TransformedCard& card) {
    bind_and_step_card(card);
}

void Loader::load_print(const TransformedPrint& print) {
    bind_and_step_print(print);
}

void Loader::load_cards(const std::vector<TransformedCard>& cards) {
    begin_transaction();
    try {
        for (const auto& c : cards)
            bind_and_step_card(c);
        commit_transaction();
    } catch (...) {
        rollback_transaction();
        throw;
    }
}

void Loader::load_prints(const std::vector<TransformedPrint>& prints) {
    begin_transaction();
    try {
        for (const auto& p : prints)
            bind_and_step_print(p);
        commit_transaction();
    } catch (...) {
        rollback_transaction();
        throw;
    }
}

// ============================================================
//  Error helper
// ============================================================

void Loader::throw_sqlite_error(const std::string& context) const {
    throw LoaderError(fmt::format("{}: {}", context, sqlite3_errmsg(db_)));
}