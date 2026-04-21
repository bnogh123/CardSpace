#pragma once
#include "scryfall_client.hpp"
#include "transformer.hpp"
#include "warehouse.hpp"
#include <string>
#include <nlohmann/json.hpp>

class ETLPipeline {
public:
    explicit ETLPipeline(const std::string& db_path);

    /**
     * @brief Run the ETL pipeline.
     * @param oracle_url URL for Oracle bulk JSON data.
     * @param all_cards_url URL for All-Cards bulk JSON data.
     */
    void run(const std::string& oracle_url, const std::string& all_cards_url);

private:
    ScryfallClient scryfall_client_;
    Warehouse warehouse_;

    /**
     * @brief Fetch and parse Oracle cards.
     * @param url URL for Oracle bulk JSON data.
     * @return Vector of TransformedCard objects.
     */
    std::vector<TransformedCard> fetch_and_parse_oracle_cards(const std::string& url);

    /**
     * @brief Fetch and parse Printed cards.
     * @param url URL for All-Cards bulk JSON data.
     * @return Vector of TransformedPrint objects.
     */
    std::vector<TransformedPrint> fetch_and_parse_printed_cards(const std::string& url);
};