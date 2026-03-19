#include "../include/card_parser.hpp"
#include "../include/nlohmann/json.hpp"
#include <string>
#include <vector>
#include <unordered_map>

// ============================================================
//  Lookup tables
// ============================================================

static const std::unordered_map<std::string, char> rarities = {
    {"bonus",    'b'},
    {"common",   'c'},
    {"mythic",   'm'},
    {"rare",     'r'},
    {"special",  's'},
    {"uncommon", 'u'}
};

static const std::unordered_map<std::string, uint16_t> layouts = {
    {"adventure",  6},
    {"case",      12},
    {"class",      9},
    {"flip",       1},
    {"leveler",    3},
    {"meld",      10},
    {"modal_dfc",  8},
    {"mutate",     7},
    {"normal",     0},
    {"prototype", 11},
    {"saga",       5},
    {"split",      2},
    {"transform",  4}
};

// ============================================================
//  Internal helpers
// ============================================================

static char parse_rarity(const std::string& rarity) {
    auto found = rarities.find(rarity);
    return (found != rarities.end()) ? found->second : '\0';
}

static uint16_t parse_layout(const std::string& layout) {
    auto found = layouts.find(layout);
    return (found != layouts.end()) ? found->second : 0;
}

// ============================================================
//  Element-level parsers
// ============================================================

Card parse_oracle_card(const nlohmann::json& element) {
    Card card;
    // --- guaranteed oracle fields ---
    card.o_id          = element["oracle_id"].get<std::string>();
    card.name          = element["name"].get<std::string>();
    card.mana_cost     = element["mana_cost"].get<std::string>();
    card.cmc           = element["cmc"].get<std::uint16_t>();
    card.type_line_raw = element["type_line"].get<std::string>();
    card.oracle_text   = element["oracle_text"].get<std::string>();
    card.type          = encode_type_line(card.type_line_raw);
    card.color         = encode_color(element["colors"]);
    card.color_identity = encode_color(element["color_identity"]);
    // TODO: card.legalities  = parse_legalities(element["legalities"]);
    // TODO: card.keywords    = element["keywords"].get<std::vector<std::string>>();

    // --- optional gameplay stats ---
    if (element.contains("power"))
        card.power     = element["power"].get<std::uint16_t>();
    if (element.contains("toughness"))
        card.toughness = element["toughness"].get<std::uint16_t>();
    if (element.contains("loyalty"))
        card.loyalty   = element["loyalty"].get<std::uint16_t>();
    if (element.contains("defense"))
        card.defense   = element["defense"].get<std::uint16_t>();

    return card;
}

PrintedCard parse_printed_card(const nlohmann::json& element) {
    PrintedCard card;
    // --- guaranteed printing fields ---
    card.id      = element["id"].get<std::string>();
    card.o_id    = element["oracle_id"].get<std::string>();
    card.rarity  = parse_rarity(element["rarity"].get<std::string>());
    card.layout  = parse_layout(element["layout"].get<std::string>());
    // TODO: card.usd_price       = element["prices"]["usd"].get<double>();
    // TODO: card.set             = element["set"].get<std::string>();
    // TODO: card.set_name        = element["set_name"].get<std::string>();
    // TODO: card.collector_number = element["collector_number"].get<std::string>();
    // TODO: card.artist          = element["artist"].get<std::string>();
    // TODO: card.border_color    = element["border_color"].get<std::string>();
    // TODO: card.frame           = element["frame"].get<std::string>();
    // TODO: card.released_at     = element["released_at"].get<std::string>();

    // --- boolean printing flags ---
    // TODO: card.foil            = element["foil"].get<bool>();
    // TODO: card.nonfoil         = element["nonfoil"].get<bool>();
    // TODO: card.oversized       = element["oversized"].get<bool>();
    // TODO: card.full_art        = element["full_art"].get<bool>();
    // TODO: card.textless        = element["textless"].get<bool>();
    // TODO: card.promo           = element["promo"].get<bool>();
    // TODO: card.reprint         = element["reprint"].get<bool>();
    // TODO: card.digital         = element["digital"].get<bool>();
    // TODO: card.booster         = element["booster"].get<bool>();
    // TODO: card.reserved        = element["reserved"].get<bool>();
    // TODO: card.variation       = element["variation"].get<bool>();
    // TODO: card.game_changer    = element["game_changer"].get<bool>();
    // TODO: card.story_spotlight = element["story_spotlight"].get<bool>();

    return card;
}

// ============================================================
//  Bulk parsers
// ============================================================

std::vector<Card> parse_oracle_cards(const std::string& raw_json) {
    nlohmann::json data = nlohmann::json::parse(raw_json);
    std::vector<Card> result;
    result.reserve(data.size());
    for (const nlohmann::json& element : data)
        result.push_back(parse_oracle_card(element));
    return result;
}

std::vector<PrintedCard> parse_printed_cards(const std::string& raw_json) {
    nlohmann::json data = nlohmann::json::parse(raw_json);
    std::vector<PrintedCard> result;
    result.reserve(data.size());
    for (const nlohmann::json& element : data)
        result.push_back(parse_printed_card(element));
    return result;
}