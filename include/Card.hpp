#pragma once
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include "CardType.hpp"

// Struct representing the basic information of a card.
// Notably is naive to the actual stats of the card;
// more of an abstract class than anything else, seeing how 
struct Card {
    std::string id;
    std::string name;
    std::string mana_cost;
    std::string type_line_raw;
    std::string oracle_text;
    std::string rarity; 
    double usd_price;
    int cmc;
    TypeEncoding type;
    std::vector<std::string> color;
    std::vector<std::string> color_identity;
    std::vector<std::string> legalities;

    // optional fields
    std::optional<uint16_t> power;
    std::optional<uint16_t> toughness;
    std::optional<uint16_t> loyalty;
    std::optional<uint16_t> defense;
    
    // unknown/rare fields as a catch-all
    std::unordered_map<std::string, std::string> extra_fields;
};
