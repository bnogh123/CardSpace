#pragma once
#include "Card.hpp"
#include <vector>
#include <string>

/** 
 * @brief Parse the raw string into a vector of the cards
 * @param raw_json Aggregated string of json file
 * @return Vector that contains each card object
*/
std::vector<Card> parse_cards(const std::string& raw_json);