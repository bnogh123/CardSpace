#pragma once
#include <string>
#include <vector>
#include <optional>
#include "CardType.hpp"
#include "CardColor.hpp"
#include "CardLegality.hpp"

/**
 * @brief Struct representing the basic information of a card.
 * 
 * Notably is naive to the actual physical stats of the card;
 * more of an abstract class than anything else, seeing how 
 * it can't really do anything by itself; beyond simplification
 * we need a specific PrintedCard object to make commentary on set
 * or monetary value as it contributes to a decks
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
    LegalityEncoding legalities;
    std::vector<std::string> keywords;

    // optional gameplay stats
    std::optional<uint16_t> power;
    std::optional<uint16_t> toughness;
    std::optional<uint16_t> loyalty;
    std::optional<uint16_t> defense;
};
