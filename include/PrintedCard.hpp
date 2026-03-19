#pragma once
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>



// PrintedCard.hpp — printing-specific, references a Card
struct PrintedCard {
    std::string id;             // scryfall printing ID
    std::string o_id;           // foreign key back to Card
    char rarity;
    double usd_price;
    std::unordered_map<std::string, std::string> extra_fields;
    // future: set_code, collector_number, image_uri, foil, etc.
};