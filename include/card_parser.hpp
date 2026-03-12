#pragma once
#include "Card.hpp"
#include <vector>
#include <string>

std::vector<Card> parse_cards(const std::string& raw_json);