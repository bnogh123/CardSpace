#pragma once
#include "Card.hpp"
#include <vector>
#include <string>

// input: aggregated string of json file raw_json
// output:  a massive vector that contains each card 
//          object and it's relevant encoded subfields
std::vector<Card> parse_cards(const std::string& raw_json);