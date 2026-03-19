#pragma once
#include "Card.hpp"
#include "PrintedCard.hpp"
#include <vector>
#include <string>
#include "nlohmann/json.hpp"

/**
 * @brief Parse oracle bulk JSON into a vector of Cards
 * @param raw_json Aggregated string from Scryfall oracle bulk endpoint
 * @return Vector of oracle Card objects
 */
std::vector<Card> parse_oracle_cards(const std::string& raw_json);

/**
 * @brief Parse all-cards bulk JSON into a vector of PrintedCards
 * @param raw_json Aggregated string from Scryfall all-cards bulk endpoint
 * @return Vector of PrintedCard objects
 */
std::vector<PrintedCard> parse_printed_cards(const std::string& raw_json);

/**
 * @brief Parse a single JSON element into an oracle Card
 * @param element A single card JSON object
 * @return Populated Card struct
 */
Card parse_oracle_card(const nlohmann::json& element);

/**
 * @brief Parse a single JSON element into a PrintedCard
 * @param element A single card JSON object
 * @return Populated PrintedCard struct
 */
PrintedCard parse_printed_card(const nlohmann::json& element);