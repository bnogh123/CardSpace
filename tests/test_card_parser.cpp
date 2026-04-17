#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include "Card.hpp"
#include "PrintedCard.hpp"
#include "card_parser.hpp"
#include "scryfall_client.hpp"

using json = nlohmann::json;

// ============================================================
//  File Helpers
// ============================================================

static json loadJson(const std::string& path) {
    std::ifstream file(path);
    return json::parse(file);
}

// ============================================================
//  Oracle Card Fixtures
// ============================================================

// Minimal instant — no P/T, no loyalty
static json make_bolt() {
    return {
        {"oracle_id",       "e3285e6b-3e79-4d7c-bf96-d920f973b122"},
        {"name",            "Lightning Bolt"},
        {"mana_cost",       "{R}"},
        {"cmc",             1},
        {"type_line",       "Instant"},
        {"oracle_text",     "Lightning Bolt deals 3 damage to any target."},
        {"colors",          json::array({"R"})},
        {"color_identity",  json::array({"R"})},
        {"keywords",        json::array()},
        {"legalities", {
            {"standard",  "not_legal"}, {"pioneer",   "not_legal"},
            {"modern",    "legal"},     {"legacy",    "legal"},
            {"vintage",   "legal"},     {"commander", "legal"},
            {"pauper",    "legal"},     {"brawl",     "not_legal"},
            {"historic",  "not_legal"},{"timeless",  "not_legal"}
        }}
    };
}

// Minimal creature — base P/T 2/2
static json make_grizzly() {
    return {
        {"oracle_id",       "some-uuid-bears"},
        {"name",            "Grizzly Bears"},
        {"mana_cost",       "{1}{G}"},
        {"cmc",             2},
        {"type_line",       "Creature — Bear"},
        {"oracle_text",     ""},
        {"colors",          json::array({"G"})},
        {"color_identity",  json::array({"G"})},
        {"keywords",        json::array()},
        {"power",           "2"},
        {"toughness",       "2"},
        {"legalities", {
            {"standard",  "not_legal"}, {"pioneer",   "legal"},
            {"modern",    "legal"},     {"legacy",    "legal"},
            {"vintage",   "legal"},     {"commander", "legal"},
            {"pauper",    "not_legal"}, {"brawl",     "not_legal"},
            {"historic",  "legal"},     {"timeless",  "legal"}
        }}
    };
}

// ============================================================
//  PrintedCard Fixture
// ============================================================

// All required fields present; usd_price is a string as Scryfall sends it
static json make_printed_bolt() {
    return {
        {"id",               "some-print-uuid"},
        {"oracle_id",        "e3285e6b-3e79-4d7c-bf96-d920f973b122"},
        {"rarity",           "common"},
        {"layout",           "normal"},
        {"prices",           {{"usd", "0.25"}}},   // string — NOT a JSON number
        {"set",              "lea"},
        {"set_name",         "Limited Edition Alpha"},
        {"collector_number", "161"},
        {"artist",           "Christopher Rush"},
        {"border_color",     "black"},
        {"frame",            "1993"},
        {"released_at",      "1993-08-05"},
        {"foil",             false},
        {"nonfoil",          true},
        {"oversized",        false},
        {"full_art",         false},
        {"textless",         false},
        {"promo",            false},
        {"reprint",          true},
        {"digital",          false},
        {"booster",          true},
        {"reserved",         true},
        {"variation",        false},
        {"story_spotlight",  false}
    };
}

// ============================================================
//  Oracle Card — String Fields
// ============================================================

TEST_CASE("parse_oracle_card: basic string fields", "[parser][oracle]") {
    Card card = parse_oracle_card(make_bolt());

    REQUIRE(card.name          == "Lightning Bolt");
    REQUIRE(card.mana_cost     == "{R}");
    REQUIRE(card.cmc           == 1);
    REQUIRE(card.type_line_raw == "Instant");
    REQUIRE(card.o_id          == "e3285e6b-3e79-4d7c-bf96-d920f973b122");
    REQUIRE(card.oracle_text   == "Lightning Bolt deals 3 damage to any target.");
}

// Lands have no mana_cost field in Scryfall oracle JSON — parser must not throw
TEST_CASE("parse_oracle_card: missing mana_cost on land does not throw", "[parser][oracle]") {
    json j = make_bolt();
    j["type_line"] = "Basic Land — Forest";
    j["colors"]    = json::array();
    j["color_identity"] = json::array({"G"});
    j.erase("mana_cost");

    REQUIRE_NOTHROW(parse_oracle_card(j));
    Card card = parse_oracle_card(j);
    REQUIRE(card.mana_cost == "");  // should default to empty string
}

// ============================================================
//  Oracle Card — Type Encoding
// ============================================================

TEST_CASE("parse_oracle_card: type encoding for instant", "[parser][type]") {
    Card card = parse_oracle_card(make_bolt());

    REQUIRE(card.type.has(CardType::INSTANT));
    REQUIRE_FALSE(card.type.has(CardType::CREATURE));
    REQUIRE_FALSE(card.type.has(CardType::LAND));
    REQUIRE_FALSE(card.type.has(CardType::LEGENDARY));
}

TEST_CASE("parse_oracle_card: type encoding for legendary creature", "[parser][type]") {
    json j = make_grizzly();
    j["type_line"] = "Legendary Creature — Dragon";
    j["name"]      = "Test Dragon";

    Card card = parse_oracle_card(j);

    REQUIRE(card.type.has(CardType::LEGENDARY));
    REQUIRE(card.type.has(CardType::CREATURE));
    REQUIRE_FALSE(card.type.has(CardType::INSTANT));
}

TEST_CASE("parse_oracle_card: type encoding for artifact creature", "[parser][type]") {
    json j = make_grizzly();
    j["type_line"] = "Artifact Creature — Golem";

    Card card = parse_oracle_card(j);

    REQUIRE(card.type.has(CardType::ARTIFACT));
    REQUIRE(card.type.has(CardType::CREATURE));
    REQUIRE(card.type.is_multitype());
    REQUIRE_FALSE(card.type.has(CardType::ENCHANTMENT));
}

// ============================================================
//  Oracle Card — Color Encoding
// ============================================================

TEST_CASE("parse_oracle_card: mono-red color encoding", "[parser][color]") {
    Card card = parse_oracle_card(make_bolt());

    REQUIRE(card.color.has(CardColor::RED));
    REQUIRE_FALSE(card.color.has(CardColor::WHITE));
    REQUIRE_FALSE(card.color.has(CardColor::BLUE));
    REQUIRE_FALSE(card.color.is_multicolor());
}

TEST_CASE("parse_oracle_card: multicolor encoding", "[parser][color]") {
    json j = make_grizzly();
    j["colors"]         = json::array({"G", "U"});
    j["color_identity"] = json::array({"G", "U"});

    Card card = parse_oracle_card(j);

    REQUIRE(card.color.has(CardColor::GREEN));
    REQUIRE(card.color.has(CardColor::BLUE));
    REQUIRE(card.color.is_multicolor());
    REQUIRE_FALSE(card.color.has(CardColor::RED));
}

TEST_CASE("parse_oracle_card: colorless encoding", "[parser][color]") {
    json j = make_bolt();
    j["colors"]         = json::array();
    j["color_identity"] = json::array();
    j["mana_cost"]      = "{3}";

    Card card = parse_oracle_card(j);

    REQUIRE(card.color.raw() == static_cast<uint16_t>(CardColor::NONE));
    REQUIRE_FALSE(card.color.is_multicolor());
}

// color_identity can differ from color (e.g. Transguild Courier)
TEST_CASE("parse_oracle_card: color identity independent of color", "[parser][color]") {
    json j = make_bolt();
    j["colors"]         = json::array();       // colorless card
    j["color_identity"] = json::array({"R"});  // but has R in rules text

    Card card = parse_oracle_card(j);

    REQUIRE_FALSE(card.color.has(CardColor::RED));
    REQUIRE(card.color_identity.has(CardColor::RED));
}

// ============================================================
//  Oracle Card — Power / Toughness Variants
// ============================================================

TEST_CASE("parse_oracle_card: BasePT for normal creature", "[parser][pt]") {
    Card card = parse_oracle_card(make_grizzly());

    REQUIRE(card.pt.has_value());
    REQUIRE(std::holds_alternative<BasePT>(card.pt.value()));

    const BasePT& base = std::get<BasePT>(card.pt.value());
    REQUIRE(base.power     == 2);
    REQUIRE(base.toughness == 2);
}

TEST_CASE("parse_oracle_card: no P/T for non-creature", "[parser][pt]") {
    Card card = parse_oracle_card(make_bolt());
    REQUIRE_FALSE(card.pt.has_value());
}

TEST_CASE("parse_oracle_card: CDAPT when power is star", "[parser][pt]") {
    json j = make_grizzly();
    j["power"]     = "*";
    j["toughness"] = "3";

    Card card = parse_oracle_card(j);

    REQUIRE(card.pt.has_value());
    REQUIRE(std::holds_alternative<CDAPT>(card.pt.value()));

    const CDAPT& cda = std::get<CDAPT>(card.pt.value());
    REQUIRE((cda.cda_flags & 0b01) != 0);  // power has CDA
    REQUIRE((cda.cda_flags & 0b10) == 0);  // toughness does not
    REQUIRE(cda.toughness == 3);
}

TEST_CASE("parse_oracle_card: CDAPT when toughness is star", "[parser][pt]") {
    json j = make_grizzly();
    j["power"]     = "2";
    j["toughness"] = "*";

    Card card = parse_oracle_card(j);

    REQUIRE(std::holds_alternative<CDAPT>(card.pt.value()));
    const CDAPT& cda = std::get<CDAPT>(card.pt.value());
    REQUIRE((cda.cda_flags & 0b01) == 0);  // power does not
    REQUIRE((cda.cda_flags & 0b10) != 0);  // toughness has CDA
}

// Negative power — parser must store absolute value in NegativePT
// e.g. "-1/5" → power=1, toughness=5 (NOT 65535 from wrapping cast)
TEST_CASE("parse_oracle_card: NegativePT stores absolute power value", "[parser][pt]") {
    json j = make_grizzly();
    j["power"]     = "-1";
    j["toughness"] = "5";

    Card card = parse_oracle_card(j);

    REQUIRE(card.pt.has_value());
    REQUIRE(std::holds_alternative<NegativePT>(card.pt.value()));

    const auto& neg = std::get<NegativePT>(card.pt.value());
    REQUIRE(neg.power     == 1);  // absolute value — NOT 65535
    REQUIRE(neg.toughness == 5);
}

// ============================================================
//  Oracle Card — Legalities
// ============================================================

TEST_CASE("parse_oracle_card: legality encoding for bolt", "[parser][legality]") {
    Card card = parse_oracle_card(make_bolt());

    REQUIRE(get_format_status(card.legalities, Format::modern)    == LegalityStatus::legal);
    REQUIRE(get_format_status(card.legalities, Format::legacy)    == LegalityStatus::legal);
    REQUIRE(get_format_status(card.legalities, Format::vintage)   == LegalityStatus::legal);
    REQUIRE(get_format_status(card.legalities, Format::standard)  == LegalityStatus::illegal);
    REQUIRE(get_format_status(card.legalities, Format::pioneer)   == LegalityStatus::illegal);
}

// ============================================================
//  Oracle Card — Optional Fields
// ============================================================

TEST_CASE("parse_oracle_card: planeswalker loyalty populated", "[parser][loyalty]") {
    json j = make_bolt();
    j["type_line"] = "Legendary Planeswalker — Jace";
    j["loyalty"]   = "3";

    Card card = parse_oracle_card(j);

    REQUIRE(card.loyalty.has_value());
    REQUIRE(card.loyalty.value() == 3);
    REQUIRE_FALSE(card.pt.has_value());
}

TEST_CASE("parse_oracle_card: no loyalty on non-planeswalker", "[parser][loyalty]") {
    Card card = parse_oracle_card(make_bolt());
    REQUIRE_FALSE(card.loyalty.has_value());
}

TEST_CASE("parse_oracle_card: keywords populated", "[parser][oracle]") {
    json j = make_grizzly();
    j["keywords"] = json::array({"Trample", "Haste"});

    Card card = parse_oracle_card(j);

    REQUIRE(card.keywords.size() == 2);
    REQUIRE(card.keywords[0] == "Trample");
    REQUIRE(card.keywords[1] == "Haste");
}

// ============================================================
//  PrintedCard Parser
// ============================================================

TEST_CASE("parse_printed_card: identity fields", "[parser][printed]") {
    PrintedCard card = parse_printed_card(make_printed_bolt());

    REQUIRE(card.id   == "some-print-uuid");
    REQUIRE(card.o_id == "e3285e6b-3e79-4d7c-bf96-d920f973b122");
}

TEST_CASE("parse_printed_card: rarity encodes correctly", "[parser][printed]") {
    PrintedCard card = parse_printed_card(make_printed_bolt());
    REQUIRE(card.rarity == 'c');  // "common" -> 'c'
}

TEST_CASE("parse_printed_card: layout encodes to normal", "[parser][printed]") {
    PrintedCard card = parse_printed_card(make_printed_bolt());
    REQUIRE(card.layout == 0);  // "normal" -> 0
}

TEST_CASE("parse_printed_card: usd_price parsed from string", "[parser][printed]") {
    // Scryfall sends prices as strings — parser must stod, not get<double>()
    PrintedCard card = parse_printed_card(make_printed_bolt());

    REQUIRE(card.usd_price.has_value());
    REQUIRE(card.usd_price.value() == Catch::Approx(0.25));
}

TEST_CASE("parse_printed_card: null usd_price becomes nullopt", "[parser][printed]") {
    json j = make_printed_bolt();
    j["prices"]["usd"] = nullptr;

    PrintedCard card = parse_printed_card(j);

    REQUIRE_FALSE(card.usd_price.has_value());
}

TEST_CASE("parse_printed_card: boolean flags", "[parser][printed]") {
    PrintedCard card = parse_printed_card(make_printed_bolt());

    REQUIRE_FALSE(card.foil);
    REQUIRE(card.nonfoil);
    REQUIRE(card.reprint);
    REQUIRE(card.reserved);
    REQUIRE_FALSE(card.digital);
    REQUIRE_FALSE(card.promo);
    REQUIRE_FALSE(card.full_art);
}

TEST_CASE("parse_printed_card: set metadata", "[parser][printed]") {
    PrintedCard card = parse_printed_card(make_printed_bolt());

    REQUIRE(card.set          == "lea");
    REQUIRE(card.set_name     == "Limited Edition Alpha");
    REQUIRE(card.released_at  == "1993-08-05");
    REQUIRE(card.artist       == "Christopher Rush");
    REQUIRE(card.frame        == "1993");
    REQUIRE(card.border_color == "black");
}

// ============================================================
//  Bulk Parser Integration Tests
// ============================================================

TEST_CASE("parse_oracle_cards: parses array into correct-size vector", "[parser][bulk]") {
    json bulk  = json::array({make_bolt(), make_grizzly()});
    auto cards = parse_oracle_cards(bulk.dump());

    REQUIRE(cards.size() == 2);
}

TEST_CASE("parse_oracle_cards: card names preserved in order", "[parser][bulk]") {
    json bulk  = json::array({make_bolt(), make_grizzly()});
    auto cards = parse_oracle_cards(bulk.dump());

    REQUIRE(cards[0].name == "Lightning Bolt");
    REQUIRE(cards[1].name == "Grizzly Bears");
}

TEST_CASE("parse_oracle_cards: empty array returns empty vector", "[parser][bulk]") {
    auto cards = parse_oracle_cards("[]");
    REQUIRE(cards.empty());
}

TEST_CASE("parse_printed_cards: parses array into correct-size vector", "[parser][bulk]") {
    json bulk  = json::array({make_printed_bolt()});
    auto cards = parse_printed_cards(bulk.dump());

    REQUIRE(cards.size() == 1);
    REQUIRE(cards[0].id == "some-print-uuid");
}

// ============================================================
//  Live Integration Tests (run with: ./cardspace_tests "[live]")
// ============================================================

TEST_CASE("ScryfallClient + parse_oracle_card: live Lightning Bolt", "[live][integration]") {
    ScryfallClient client;
    std::string raw = client.fetch_bulk_data(
        "https://api.scryfall.com/cards/named?exact=Lightning+Bolt&format=json"
    );

    auto j = json::parse(raw);

    std::cout << j.dump(2) << std::endl;

    // The /cards/named endpoint returns a printing object, not an oracle object.
    // It has oracle_id but may be missing mana_cost on some layouts — use value()
    REQUIRE(j.contains("oracle_id"));
    REQUIRE(j.contains("name"));

    auto card = parse_oracle_card(j);

    REQUIRE(card.name == "Lightning Bolt");
    REQUIRE(card.type.has(CardType::INSTANT));
    REQUIRE(card.color.has(CardColor::RED));
    REQUIRE(get_format_status(card.legalities, Format::modern) == LegalityStatus::legal);
}