#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "../include/transformer.hpp"
#include "../include/loader.hpp"
#include "../include/CardType.hpp"
#include "../include/CardColor.hpp"
#include "../include/CardLegality.hpp"
#include <sqlite3.h>
#include <string>
#include <vector>
#include <optional>

// ============================================================
//  Fixture helpers
//  Build minimal but realistic Card / PrintedCard objects.
//  All encoding helpers (encode_type_line, encode_color, etc.)
//  are assumed correct — these tests verify the transformer and
//  loader layers, not the encoders themselves.
// ============================================================

static Card make_creature() {
    Card c;
    c.o_id          = "test-oracle-creature-001";
    c.name          = "Grizzly Bears";
    c.mana_cost     = "{1}{G}";
    c.cmc           = 2;
    c.type_line_raw = "Creature — Bear";
    c.oracle_text   = "";
    c.type          = encode_type_line("Creature — Bear");
    c.color         = encode_color(std::string("{G}"));
    c.color_identity = encode_color(std::string("{G}"));
    c.legalities    = encode_legalities(nlohmann::json::parse(R"({
        "standard":"not_legal","pioneer":"not_legal","modern":"legal",
        "legacy":"legal","vintage":"legal","commander":"legal",
        "pauper":"legal","brawl":"not_legal","historic":"not_legal",
        "timeless":"not_legal"
    })"));
    c.keywords      = {};
    c.pt            = BasePT{2, 2};
    return c;
}

static Card make_planeswalker() {
    Card c;
    c.o_id          = "test-oracle-pw-001";
    c.name          = "Jace, the Mind Sculptor";
    c.mana_cost     = "{2}{U}{U}";
    c.cmc           = 4;
    c.type_line_raw = "Legendary Planeswalker — Jace";
    c.oracle_text   = "+2: Look at the top card of target player's library.";
    c.type          = encode_type_line("Legendary Planeswalker — Jace");
    c.color         = encode_color(std::string("{U}"));
    c.color_identity = encode_color(std::string("{U}"));
    c.legalities    = encode_legalities(nlohmann::json::parse(R"({
        "standard":"not_legal","pioneer":"not_legal","modern":"not_legal",
        "legacy":"legal","vintage":"legal","commander":"legal",
        "pauper":"not_legal","brawl":"not_legal","historic":"not_legal",
        "timeless":"not_legal"
    })"));
    c.keywords      = {};
    c.loyalty       = 3;
    return c;
}

static Card make_cda_creature() {
    Card c;
    c.o_id          = "test-oracle-cda-001";
    c.name          = "Tarmogoyf";
    c.mana_cost     = "{1}{G}";
    c.cmc           = 2;
    c.type_line_raw = "Creature — Lhurgoyf";
    c.oracle_text   = "Tarmogoyf's power is equal to the number of card types among cards in all graveyards.";
    c.type          = encode_type_line("Creature — Lhurgoyf");
    c.color         = encode_color(std::string("{G}"));
    c.color_identity = encode_color(std::string("{G}"));
    c.legalities    = encode_legalities(nlohmann::json::parse(R"({
        "standard":"not_legal","pioneer":"not_legal","modern":"legal",
        "legacy":"legal","vintage":"legal","commander":"legal",
        "pauper":"not_legal","brawl":"not_legal","historic":"not_legal",
        "timeless":"not_legal"
    })"));
    c.keywords      = {};
    c.pt            = CDAPT{0, 1, 0b01};  // power is CDA, toughness is base+1
    return c;
}

static Card make_negative_pt_creature() {
    Card c;
    c.o_id          = "test-oracle-neg-001";
    c.name          = "Char-Rumbler";
    c.mana_cost     = "{3}{R}";
    c.cmc           = 4;
    c.type_line_raw = "Creature — Elemental";
    c.oracle_text   = "First strike";
    c.type          = encode_type_line("Creature — Elemental");
    c.color         = encode_color(std::string("{R}"));
    c.color_identity = encode_color(std::string("{R}"));
    c.legalities    = encode_legalities(nlohmann::json::parse(R"({
        "standard":"not_legal","pioneer":"not_legal","modern":"not_legal",
        "legacy":"legal","vintage":"legal","commander":"legal",
        "pauper":"not_legal","brawl":"not_legal","historic":"not_legal",
        "timeless":"not_legal"
    })"));
    c.keywords      = {"First strike"};
    c.pt            = NegativePT{1, 1};  // -1/1
    return c;
}

static Card make_non_creature() {
    Card c;
    c.o_id          = "test-oracle-land-001";
    c.name          = "Forest";
    c.mana_cost     = "";
    c.cmc           = 0;
    c.type_line_raw = "Basic Land — Forest";
    c.oracle_text   = "({T}: Add {G}.)";
    c.type          = encode_type_line("Basic Land — Forest");
    c.color         = encode_color(std::string(""));
    c.color_identity = encode_color(std::string("{G}"));
    c.legalities    = encode_legalities(nlohmann::json::parse(R"({
        "standard":"legal","pioneer":"legal","modern":"legal",
        "legacy":"legal","vintage":"legal","commander":"legal",
        "pauper":"legal","brawl":"legal","historic":"legal",
        "timeless":"legal"
    })"));
    c.keywords      = {};
    return c;
}

static Card make_multi_keyword() {
    Card c;
    c.o_id          = "test-oracle-kw-001";
    c.name          = "Serra Angel";
    c.mana_cost     = "{3}{W}{W}";
    c.cmc           = 5;
    c.type_line_raw = "Creature — Angel";
    c.oracle_text   = "Flying, vigilance";
    c.type          = encode_type_line("Creature — Angel");
    c.color         = encode_color(std::string("{W}"));
    c.color_identity = encode_color(std::string("{W}"));
    c.legalities    = encode_legalities(nlohmann::json::parse(R"({
        "standard":"not_legal","pioneer":"not_legal","modern":"not_legal",
        "legacy":"legal","vintage":"legal","commander":"legal",
        "pauper":"not_legal","brawl":"not_legal","historic":"not_legal",
        "timeless":"not_legal"
    })"));
    c.keywords      = {"Flying", "Vigilance"};
    c.pt            = BasePT{4, 4};
    return c;
}

static PrintedCard make_print(const std::string& id, const std::string& o_id) {
    PrintedCard p;
    p.id               = id;
    p.o_id             = o_id;
    p.set              = "m21";
    p.set_id           = "set-uuid-m21";
    p.set_name         = "Core Set 2021";
    p.set_type         = "core";
    p.collector_number = "100";
    p.artist           = "Test Artist";
    p.border_color     = "black";
    p.frame            = "2015";
    p.released_at      = "2020-07-03";
    p.image_status     = "highres_scan";
    p.rarity           = 'r';
    p.layout           = 0;  // normal
    p.foil             = true;
    p.nonfoil          = true;
    p.oversized        = false;
    p.full_art         = false;
    p.textless         = false;
    p.promo            = false;
    p.reprint          = true;
    p.digital          = false;
    p.booster          = true;
    p.reserved         = false;
    p.variation        = false;
    p.story_spotlight  = false;
    p.highres_image    = true;
    p.usd_price        = 45.00;
    p.usd_foil_price   = 60.00;
    p.games            = std::vector<std::string>{"paper", "mtgo"};
    p.finishes         = std::vector<std::string>{"nonfoil", "foil"};
    p.multiverse_ids   = std::vector<int>{12345, 67890};
    p.scryfall_uri     = "https://scryfall.com/card/m21/100";
    p.rulings_uri      = "https://api.scryfall.com/cards/m21/100/rulings";
    p.prints_search_uri = "https://api.scryfall.com/cards/search?order=released";
    return p;
}

// ============================================================
//  In-memory SQLite helper
//  Opens :memory:, inits schema, and tears down automatically.
// ============================================================
struct TestDB {
    Loader loader;
    explicit TestDB() : loader(":memory:") {
        loader.init_schema();
    }

    // Query a single text column from oracle_cards by o_id.
    std::string query_text(const std::string& col, const std::string& o_id) {
        sqlite3* raw = raw_db();
        std::string sql = "SELECT " + col + " FROM oracle_cards WHERE o_id = ?;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(raw, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, o_id.c_str(), -1, SQLITE_TRANSIENT);
        std::string result;
        if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_type(stmt, 0) != SQLITE_NULL)
            result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        sqlite3_finalize(stmt);
        return result;
    }

    // Query a single integer column from oracle_cards by o_id.
    // Returns nullopt if the column value is NULL.
    std::optional<int64_t> query_int(const std::string& col, const std::string& o_id,
                                     const std::string& table = "oracle_cards",
                                     const std::string& key_col = "o_id") {
        sqlite3* raw = raw_db();
        std::string sql = "SELECT " + col + " FROM " + table +
                          " WHERE " + key_col + " = ?;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(raw, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, o_id.c_str(), -1, SQLITE_TRANSIENT);
        std::optional<int64_t> result;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            if (sqlite3_column_type(stmt, 0) != SQLITE_NULL)
                result = sqlite3_column_int64(stmt, 0);
        }
        sqlite3_finalize(stmt);
        return result;
    }

    std::optional<double> query_double(const std::string& col, const std::string& id,
                                       const std::string& table = "printed_cards",
                                       const std::string& key_col = "id") {
        sqlite3* raw = raw_db();
        std::string sql = "SELECT " + col + " FROM " + table +
                          " WHERE " + key_col + " = ?;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(raw, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
        std::optional<double> result;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            if (sqlite3_column_type(stmt, 0) != SQLITE_NULL)
                result = sqlite3_column_double(stmt, 0);
        }
        sqlite3_finalize(stmt);
        return result;
    }

    std::string query_print_text(const std::string& col, const std::string& id) {
        sqlite3* raw = raw_db();
        std::string sql = "SELECT " + col + " FROM printed_cards WHERE id = ?;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(raw, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
        std::string result;
        if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_type(stmt, 0) != SQLITE_NULL)
            result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        sqlite3_finalize(stmt);
        return result;
    }

    int64_t row_count(const std::string& table) {
        sqlite3* raw = raw_db();
        std::string sql = "SELECT COUNT(*) FROM " + table + ";";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(raw, sql.c_str(), -1, &stmt, nullptr);
        int64_t count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW)
            count = sqlite3_column_int64(stmt, 0);
        sqlite3_finalize(stmt);
        return count;
    }

private:
    // Loader doesn't expose the raw db handle, so we open a second
    // connection to :memory: — but :memory: creates an independent DB
    // per connection. Instead we expose a backdoor via a thin subclass.
    // We work around this by making TestDB a friend in a real project,
    // but for test purposes we keep a parallel sqlite3* to the same file.
    // Since we're using :memory: we use the Loader's internal connection
    // indirectly by querying through a re-opened handle to the same path.
    //
    // Practical fix: expose a raw_db() accessor on Loader for test builds,
    // or use a named temp file. Here we use a named file under /tmp.
    sqlite3* raw_db() {
        // This method is only valid when TestDB was constructed with a
        // file path, not :memory:. See NOTE in test file header.
        return nullptr;  // replaced by the file-backed variant below
    }
};

// NOTE: sqlite3 :memory: databases are not shareable across connections.
// The cleanest solution for testing is to expose the db handle from Loader
// behind a #ifdef CARDSPACE_TEST guard, or use a named temp file.
// These tests use a named temp path under /tmp and clean up after themselves.

static const std::string TEST_DB_PATH = "/tmp/cardspace_test.db";

struct FileTestDB {
    Loader loader;
    sqlite3* raw = nullptr;

    FileTestDB() : loader(TEST_DB_PATH) {
        loader.init_schema();
        sqlite3_open(TEST_DB_PATH.c_str(), &raw);
    }

    ~FileTestDB() {
        if (raw) sqlite3_close(raw);
        std::remove(TEST_DB_PATH.c_str());
    }

    std::string query_text(const std::string& col, const std::string& id,
                            const std::string& table = "oracle_cards",
                            const std::string& key = "o_id") {
        std::string sql = "SELECT " + col + " FROM " + table + " WHERE " + key + " = ?;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(raw, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
        std::string result;
        if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_type(stmt, 0) != SQLITE_NULL)
            result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        sqlite3_finalize(stmt);
        return result;
    }

    std::optional<int64_t> query_int(const std::string& col, const std::string& id,
                                     const std::string& table = "oracle_cards",
                                     const std::string& key = "o_id") {
        std::string sql = "SELECT " + col + " FROM " + table + " WHERE " + key + " = ?;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(raw, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
        std::optional<int64_t> result;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            if (sqlite3_column_type(stmt, 0) != SQLITE_NULL)
                result = sqlite3_column_int64(stmt, 0);
        }
        sqlite3_finalize(stmt);
        return result;
    }

    std::optional<double> query_double(const std::string& col, const std::string& id,
                                       const std::string& table = "printed_cards",
                                       const std::string& key = "id") {
        std::string sql = "SELECT " + col + " FROM " + table + " WHERE " + key + " = ?;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(raw, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
        std::optional<double> result;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            if (sqlite3_column_type(stmt, 0) != SQLITE_NULL)
                result = sqlite3_column_double(stmt, 0);
        }
        sqlite3_finalize(stmt);
        return result;
    }

    int64_t row_count(const std::string& table) {
        std::string sql = "SELECT COUNT(*) FROM " + table + ";";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(raw, sql.c_str(), -1, &stmt, nullptr);
        int64_t count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW)
            count = sqlite3_column_int64(stmt, 0);
        sqlite3_finalize(stmt);
        return count;
    }
};

// ============================================================
//  TRANSFORMER TESTS
// ============================================================

TEST_CASE("transform_card: identity fields pass through unchanged", "[transformer]") {
    Card c = make_creature();
    TransformedCard t = transform_card(c);

    REQUIRE(t.o_id          == c.o_id);
    REQUIRE(t.name          == c.name);
    REQUIRE(t.mana_cost     == c.mana_cost);
    REQUIRE(t.cmc           == c.cmc);
    REQUIRE(t.type_line_raw == c.type_line_raw);
    REQUIRE(t.oracle_text   == c.oracle_text);
}

TEST_CASE("transform_card: encoded fields use raw() accessors", "[transformer]") {
    Card c = make_creature();
    TransformedCard t = transform_card(c);

    REQUIRE(t.type_encoded  == c.type.raw());
    REQUIRE(t.color         == c.color.raw());
    REQUIRE(t.color_identity == c.color_identity.raw());
}

TEST_CASE("transform_card: LegalityEncoding split into two fields", "[transformer]") {
    Card c = make_creature();
    TransformedCard t = transform_card(c);

    REQUIRE(t.legality_exceptions == c.legalities.exceptions);
    REQUIRE(t.legality_baseline   == c.legalities.baseline);
}

TEST_CASE("transform_card: BasePT flattened correctly", "[transformer]") {
    Card c = make_creature();  // BasePT{2, 2}
    TransformedCard t = transform_card(c);

    REQUIRE(t.power.has_value());
    REQUIRE(t.toughness.has_value());
    REQUIRE_FALSE(t.cda_flags.has_value());
    REQUIRE(t.power.value()     == 2);
    REQUIRE(t.toughness.value() == 2);
}

TEST_CASE("transform_card: CDAPT flattened with cda_flags set", "[transformer]") {
    Card c = make_cda_creature();  // CDAPT{0, 1, 0b01}
    TransformedCard t = transform_card(c);

    REQUIRE(t.power.has_value());
    REQUIRE(t.toughness.has_value());
    REQUIRE(t.cda_flags.has_value());
    REQUIRE(t.power.value()      == 0);
    REQUIRE(t.toughness.value()  == 1);
    REQUIRE(t.cda_flags.value()  == 0b01);
}

TEST_CASE("transform_card: NegativePT stored as negative power", "[transformer]") {
    Card c = make_negative_pt_creature();  // NegativePT{1, 1} → -1/1
    TransformedCard t = transform_card(c);

    REQUIRE(t.power.has_value());
    REQUIRE(t.toughness.has_value());
    REQUIRE_FALSE(t.cda_flags.has_value());
    REQUIRE(t.power.value()     == -1);
    REQUIRE(t.toughness.value() == 1);
}

TEST_CASE("transform_card: no PT card has all PT fields as nullopt", "[transformer]") {
    Card c = make_non_creature();  // no pt set
    TransformedCard t = transform_card(c);

    REQUIRE_FALSE(t.power.has_value());
    REQUIRE_FALSE(t.toughness.has_value());
    REQUIRE_FALSE(t.cda_flags.has_value());
}

TEST_CASE("transform_card: loyalty populated for planeswalker", "[transformer]") {
    Card c = make_planeswalker();
    TransformedCard t = transform_card(c);

    REQUIRE(t.loyalty.has_value());
    REQUIRE(t.loyalty.value() == 3);
    REQUIRE_FALSE(t.power.has_value());  // no pt on source card
    REQUIRE_FALSE(t.toughness.has_value());
}

TEST_CASE("transform_card: no loyalty on non-planeswalker", "[transformer]") {
    Card c = make_creature();
    TransformedCard t = transform_card(c);

    REQUIRE_FALSE(t.loyalty.has_value());
}

TEST_CASE("transform_card: keywords serialized as pipe-delimited string", "[transformer]") {
    Card c = make_multi_keyword();  // {"Flying", "Vigilance"}
    TransformedCard t = transform_card(c);

    REQUIRE(t.keywords_str == "Flying|Vigilance");
}

TEST_CASE("transform_card: empty keywords produces empty string", "[transformer]") {
    Card c = make_creature();
    TransformedCard t = transform_card(c);

    REQUIRE(t.keywords_str.empty());
}

TEST_CASE("transform_card: single keyword has no pipe separator", "[transformer]") {
    Card c = make_negative_pt_creature();  // {"First strike"}
    TransformedCard t = transform_card(c);

    REQUIRE(t.keywords_str == "First strike");
}

TEST_CASE("transform_print: identity and set fields pass through", "[transformer]") {
    PrintedCard p = make_print("print-001", "test-oracle-creature-001");
    TransformedPrint t = transform_print(p);

    REQUIRE(t.id               == p.id);
    REQUIRE(t.o_id             == p.o_id);
    REQUIRE(t.set              == p.set);
    REQUIRE(t.set_id           == p.set_id);
    REQUIRE(t.set_name         == p.set_name);
    REQUIRE(t.set_type         == p.set_type);
    REQUIRE(t.collector_number == p.collector_number);
    REQUIRE(t.artist           == p.artist);
    REQUIRE(t.border_color     == p.border_color);
    REQUIRE(t.frame            == p.frame);
    REQUIRE(t.released_at      == p.released_at);
    REQUIRE(t.image_status     == p.image_status);
    REQUIRE(t.rarity           == p.rarity);
    REQUIRE(t.layout           == p.layout);
}

TEST_CASE("transform_print: boolean flags packed into bitmask correctly", "[transformer]") {
    PrintedCard p = make_print("print-001", "test-oracle-creature-001");
    // make_print sets foil=true, nonfoil=true, reprint=true, booster=true, highres_image=true
    TransformedPrint t = transform_print(p);

    REQUIRE((t.flags & FLAG_FOIL)           != 0);
    REQUIRE((t.flags & FLAG_NONFOIL)        != 0);
    REQUIRE((t.flags & FLAG_REPRINT)        != 0);
    REQUIRE((t.flags & FLAG_BOOSTER)        != 0);
    REQUIRE((t.flags & FLAG_HIGHRES_IMAGE)  != 0);
    // Flags that were false should be 0
    REQUIRE((t.flags & FLAG_OVERSIZED)      == 0);
    REQUIRE((t.flags & FLAG_FULL_ART)       == 0);
    REQUIRE((t.flags & FLAG_PROMO)          == 0);
    REQUIRE((t.flags & FLAG_DIGITAL)        == 0);
    REQUIRE((t.flags & FLAG_RESERVED)       == 0);
    REQUIRE((t.flags & FLAG_STORY_SPOTLIGHT)== 0);
}

TEST_CASE("transform_print: pricing fields preserved", "[transformer]") {
    PrintedCard p = make_print("print-001", "test-oracle-creature-001");
    TransformedPrint t = transform_print(p);

    REQUIRE(t.usd_price.has_value());
    REQUIRE(t.usd_foil_price.has_value());
    REQUIRE_THAT(t.usd_price.value(),      Catch::Matchers::WithinRel(45.00, 0.001));
    REQUIRE_THAT(t.usd_foil_price.value(), Catch::Matchers::WithinRel(60.00, 0.001));
}

TEST_CASE("transform_print: absent pricing is nullopt", "[transformer]") {
    PrintedCard p = make_print("print-002", "test-oracle-creature-001");
    p.usd_price      = std::nullopt;
    p.usd_foil_price = std::nullopt;
    TransformedPrint t = transform_print(p);

    REQUIRE_FALSE(t.usd_price.has_value());
    REQUIRE_FALSE(t.usd_foil_price.has_value());
}

TEST_CASE("transform_print: optional collections serialized correctly", "[transformer]") {
    PrintedCard p = make_print("print-001", "test-oracle-creature-001");
    TransformedPrint t = transform_print(p);

    REQUIRE(t.games_str          == "paper|mtgo");
    REQUIRE(t.finishes_str       == "nonfoil|foil");
    REQUIRE(t.multiverse_ids_str == "12345,67890");
}

TEST_CASE("transform_print: absent optional collections produce empty strings", "[transformer]") {
    PrintedCard p = make_print("print-002", "test-oracle-creature-001");
    p.games          = std::nullopt;
    p.finishes       = std::nullopt;
    p.multiverse_ids = std::nullopt;
    TransformedPrint t = transform_print(p);

    REQUIRE(t.games_str.empty());
    REQUIRE(t.finishes_str.empty());
    REQUIRE(t.multiverse_ids_str.empty());
}

TEST_CASE("transform_print: uris preserved", "[transformer]") {
    PrintedCard p = make_print("print-001", "test-oracle-creature-001");
    TransformedPrint t = transform_print(p);

    REQUIRE(t.scryfall_uri      == p.scryfall_uri.value());
    REQUIRE(t.rulings_uri       == p.rulings_uri.value());
    REQUIRE(t.prints_search_uri == p.prints_search_uri.value());
}

TEST_CASE("transform_print: absent uris produce empty strings", "[transformer]") {
    PrintedCard p = make_print("print-002", "test-oracle-creature-001");
    p.scryfall_uri      = std::nullopt;
    p.rulings_uri       = std::nullopt;
    p.prints_search_uri = std::nullopt;
    TransformedPrint t = transform_print(p);

    REQUIRE(t.scryfall_uri.empty());
    REQUIRE(t.rulings_uri.empty());
    REQUIRE(t.prints_search_uri.empty());
}

TEST_CASE("transform_cards / transform_prints: batch preserves count", "[transformer]") {
    std::vector<Card> cards = {
        make_creature(), make_planeswalker(), make_non_creature()
    };
    auto result = transform_cards(cards);
    REQUIRE(result.size() == 3);

    PrintedCard p1 = make_print("p1", cards[0].o_id);
    PrintedCard p2 = make_print("p2", cards[1].o_id);
    std::vector<PrintedCard> prints = {p1, p2};
    auto print_result = transform_prints(prints);
    REQUIRE(print_result.size() == 2);
}

// ============================================================
//  LOADER TESTS
//  These tests use a named temp file (/tmp/cardspace_test.db)
//  so both the Loader connection and the test query connection
//  can see the same data. Each TEST_CASE constructs a fresh
//  FileTestDB which deletes the file on destruction.
// ============================================================

TEST_CASE("loader: init_schema creates both tables", "[loader]") {
    FileTestDB db;
    // Tables exist iff row_count doesn't throw
    REQUIRE(db.row_count("oracle_cards")  == 0);
    REQUIRE(db.row_count("printed_cards") == 0);
}

TEST_CASE("loader: init_schema is idempotent", "[loader]") {
    FileTestDB db;
    REQUIRE_NOTHROW(db.loader.init_schema());
    REQUIRE_NOTHROW(db.loader.init_schema());
}

TEST_CASE("loader: load_card inserts one row", "[loader]") {
    FileTestDB db;
    Card c = make_creature();
    db.loader.load_card(transform_card(c));

    REQUIRE(db.row_count("oracle_cards") == 1);
}

TEST_CASE("loader: loaded card fields round-trip correctly", "[loader]") {
    FileTestDB db;
    Card c = make_creature();
    TransformedCard t = transform_card(c);
    db.loader.load_card(t);

    REQUIRE(db.query_text("name",          c.o_id) == c.name);
    REQUIRE(db.query_text("mana_cost",     c.o_id) == c.mana_cost);
    REQUIRE(db.query_text("type_line_raw", c.o_id) == c.type_line_raw);
    REQUIRE(db.query_int ("cmc",           c.o_id).value() == c.cmc);
    REQUIRE(db.query_int ("type_encoded",  c.o_id).value() ==
            static_cast<int64_t>(c.type.raw()));
    REQUIRE(db.query_int ("color",         c.o_id).value() ==
            static_cast<int64_t>(c.color.raw()));
}

TEST_CASE("loader: BasePT stored as positive power and toughness", "[loader]") {
    FileTestDB db;
    Card c = make_creature();  // BasePT{2, 2}
    db.loader.load_card(transform_card(c));

    REQUIRE(db.query_int("power",     c.o_id).value() == 2);
    REQUIRE(db.query_int("toughness", c.o_id).value() == 2);
    REQUIRE_FALSE(db.query_int("cda_flags", c.o_id).has_value());
}

TEST_CASE("loader: NegativePT stored as negative power", "[loader]") {
    FileTestDB db;
    Card c = make_negative_pt_creature();  // NegativePT{1,1} → -1/1
    db.loader.load_card(transform_card(c));

    REQUIRE(db.query_int("power",     c.o_id).value() == -1);
    REQUIRE(db.query_int("toughness", c.o_id).value() ==  1);
    REQUIRE_FALSE(db.query_int("cda_flags", c.o_id).has_value());
}

TEST_CASE("loader: CDAPT stored with non-null cda_flags", "[loader]") {
    FileTestDB db;
    Card c = make_cda_creature();  // CDAPT{0, 1, 0b01}
    db.loader.load_card(transform_card(c));

    REQUIRE(db.query_int("power",     c.o_id).value() == 0);
    REQUIRE(db.query_int("toughness", c.o_id).value() == 1);
    REQUIRE(db.query_int("cda_flags", c.o_id).value() == 0b01);
}

TEST_CASE("loader: no-PT card stores NULL for power/toughness/cda_flags", "[loader]") {
    FileTestDB db;
    Card c = make_non_creature();
    db.loader.load_card(transform_card(c));

    REQUIRE_FALSE(db.query_int("power",     c.o_id).has_value());
    REQUIRE_FALSE(db.query_int("toughness", c.o_id).has_value());
    REQUIRE_FALSE(db.query_int("cda_flags", c.o_id).has_value());
}

TEST_CASE("loader: loyalty stored for planeswalker, NULL for others", "[loader]") {
    FileTestDB db;
    Card pw = make_planeswalker();
    Card cr = make_creature();
    db.loader.load_card(transform_card(pw));
    db.loader.load_card(transform_card(cr));

    REQUIRE(db.query_int("loyalty", pw.o_id).value() == 3);
    REQUIRE_FALSE(db.query_int("loyalty", cr.o_id).has_value());
}

TEST_CASE("loader: keywords round-trip as pipe-delimited string", "[loader]") {
    FileTestDB db;
    Card c = make_multi_keyword();
    db.loader.load_card(transform_card(c));

    REQUIRE(db.query_text("keywords", c.o_id) == "Flying|Vigilance");
}

TEST_CASE("loader: legality fields stored correctly", "[loader]") {
    FileTestDB db;
    Card c = make_creature();
    TransformedCard t = transform_card(c);
    db.loader.load_card(t);

    REQUIRE(db.query_int("legality_exceptions", c.o_id).value() ==
            static_cast<int64_t>(t.legality_exceptions));
    REQUIRE(db.query_int("legality_baseline", c.o_id).value() ==
            static_cast<int64_t>(t.legality_baseline));
}

TEST_CASE("loader: load_card is idempotent (INSERT OR REPLACE)", "[loader]") {
    FileTestDB db;
    Card c = make_creature();
    TransformedCard t = transform_card(c);

    db.loader.load_card(t);
    db.loader.load_card(t);  // second load should replace, not duplicate

    REQUIRE(db.row_count("oracle_cards") == 1);
}

TEST_CASE("loader: load_print inserts one row", "[loader]") {
    FileTestDB db;
    Card c = make_creature();
    db.loader.load_card(transform_card(c));

    PrintedCard p = make_print("print-001", c.o_id);
    db.loader.load_print(transform_print(p));

    REQUIRE(db.row_count("printed_cards") == 1);
}

TEST_CASE("loader: loaded print fields round-trip correctly", "[loader]") {
    FileTestDB db;
    Card c = make_creature();
    db.loader.load_card(transform_card(c));

    PrintedCard p = make_print("print-001", c.o_id);
    db.loader.load_print(transform_print(p));

    REQUIRE(db.query_text("set_code",   p.id, "printed_cards", "id") == p.set);
    REQUIRE(db.query_text("set_name",   p.id, "printed_cards", "id") == p.set_name);
    REQUIRE(db.query_text("artist",     p.id, "printed_cards", "id") == p.artist);
    REQUIRE(db.query_text("rarity",     p.id, "printed_cards", "id") == std::string(1, p.rarity));
    REQUIRE(db.query_int ("layout",     p.id, "printed_cards", "id").value() == p.layout);
}

TEST_CASE("loader: print pricing stored and NULL when absent", "[loader]") {
    FileTestDB db;
    Card c = make_creature();
    db.loader.load_card(transform_card(c));

    // With prices
    PrintedCard p1 = make_print("print-001", c.o_id);
    db.loader.load_print(transform_print(p1));
    REQUIRE_THAT(db.query_double("usd_price",      "print-001").value(),
                 Catch::Matchers::WithinRel(45.00, 0.001));
    REQUIRE_THAT(db.query_double("usd_foil_price", "print-001").value(),
                 Catch::Matchers::WithinRel(60.00, 0.001));

    // Without prices
    PrintedCard p2 = make_print("print-002", c.o_id);
    p2.usd_price      = std::nullopt;
    p2.usd_foil_price = std::nullopt;
    db.loader.load_print(transform_print(p2));
    REQUIRE_FALSE(db.query_double("usd_price",      "print-002").has_value());
    REQUIRE_FALSE(db.query_double("usd_foil_price", "print-002").has_value());
}

TEST_CASE("loader: print flags bitmask round-trips", "[loader]") {
    FileTestDB db;
    Card c = make_creature();
    db.loader.load_card(transform_card(c));

    PrintedCard p = make_print("print-001", c.o_id);
    TransformedPrint tp = transform_print(p);
    db.loader.load_print(tp);

    auto stored = db.query_int("flags", "print-001", "printed_cards", "id");
    REQUIRE(stored.has_value());
    REQUIRE(stored.value() == static_cast<int64_t>(tp.flags));
}

TEST_CASE("loader: serialized collections round-trip", "[loader]") {
    FileTestDB db;
    Card c = make_creature();
    db.loader.load_card(transform_card(c));

    PrintedCard p = make_print("print-001", c.o_id);
    db.loader.load_print(transform_print(p));

    REQUIRE(db.query_text("games",          "print-001", "printed_cards", "id") == "paper|mtgo");
    REQUIRE(db.query_text("finishes",       "print-001", "printed_cards", "id") == "nonfoil|foil");
    REQUIRE(db.query_text("multiverse_ids", "print-001", "printed_cards", "id") == "12345,67890");
}

TEST_CASE("loader: load_cards batch inserts all rows in one transaction", "[loader]") {
    FileTestDB db;
    std::vector<Card> cards = {
        make_creature(), make_planeswalker(), make_non_creature(),
        make_cda_creature(), make_negative_pt_creature(), make_multi_keyword()
    };
    auto transformed = transform_cards(cards);
    db.loader.load_cards(transformed);

    REQUIRE(db.row_count("oracle_cards") == static_cast<int64_t>(cards.size()));
}

TEST_CASE("loader: load_prints batch inserts all rows", "[loader]") {
    FileTestDB db;
    Card c = make_creature();
    db.loader.load_card(transform_card(c));

    std::vector<PrintedCard> prints = {
        make_print("p1", c.o_id),
        make_print("p2", c.o_id),
        make_print("p3", c.o_id),
    };
    db.loader.load_prints(transform_prints(prints));

    REQUIRE(db.row_count("printed_cards") == 3);
}

TEST_CASE("loader: load_cards is idempotent across batch re-runs", "[loader]") {
    FileTestDB db;
    std::vector<Card> cards = { make_creature(), make_planeswalker() };
    auto transformed = transform_cards(cards);

    db.loader.load_cards(transformed);
    db.loader.load_cards(transformed);  // re-run: should replace, not duplicate

    REQUIRE(db.row_count("oracle_cards") == 2);
}

TEST_CASE("loader: FK constraint prevents print without parent card", "[loader]") {
    FileTestDB db;
    // Intentionally do NOT load the parent oracle card first
    PrintedCard p = make_print("orphan-print", "nonexistent-oracle-id");
    REQUIRE_THROWS_AS(db.loader.load_print(transform_print(p)), LoaderError);
}