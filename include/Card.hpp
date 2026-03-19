#pragma once
#include <string>
#include <vector>
#include <optional>
#include "CardType.hpp"
#include "CardColor.hpp"

/**
 * @brief Struct representing the basic information of a card.
 * 
 * Notably is naive to the actual stats of the card;
 * more of an abstract class than anything else, seeing how 
 * it can't really do anything by itself if that makes sense.
*/
struct Card {
    // immutable string aspects
    std::string o_id;           // stable oracle ID
    std::string name;
    std::string mana_cost;
    std::string type_line_raw;
    std::string oracle_text;

    // statistical aspects
    uint16_t cmc;
    TypeEncoding type;
    ColorEncoding color;
    ColorEncoding color_identity;
    std::vector<std::string> legalities;

    // optional gameplay stats
    std::optional<uint16_t> power;
    std::optional<uint16_t> toughness;
    std::optional<uint16_t> loyalty;
    std::optional<uint16_t> defense;
};
