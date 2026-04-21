#include "etl_pipeline.hpp"
#include "card_parser.hpp"
#include "transformer.hpp"
#include <nlohmann/json.hpp>

ETLPipeline::ETLPipeline(const std::string& db_path) : warehouse_(db_path) {}

void ETLPipeline::run(const std::string& oracle_url, const std::string& all_cards_url) {
    // Transform vectors into transformed variants
    auto oracle_cards = fetch_and_parse_oracle_cards(oracle_url);
    auto all_cards = fetch_and_parse_printed_cards(all_cards_url);

    // Load parsed cards into the warehouse
    warehouse_.load_cards(oracle_cards);
    warehouse_.load_prints(all_cards);
}

std::vector<TransformedCard> ETLPipeline::fetch_and_parse_oracle_cards(const std::string& url) {
    // Fetch Oracle data from Scryfall
    std::string oracle_data = scryfall_client_.fetch_bulk_data(url);
    auto oracle_json = nlohmann::json::parse(oracle_data);
    auto oracle_vec = parse_oracle_cards(oracle_json.dump());

    // Transform vectors into transformed variants
    auto oracle_cards = transform_cards(oracle_vec);

    return oracle_cards;
}

std::vector<TransformedPrint> ETLPipeline::fetch_and_parse_printed_cards(const std::string& url) {
    // Fetch All-Cards data from Scryfall
    std::string all_cards_data = scryfall_client_.fetch_bulk_data(url);
    auto all_cards_json = nlohmann::json::parse(all_cards_data);
    auto all_cards_vec = parse_printed_cards(all_cards_json.dump());

    // Transform vectors into transformed variants
    auto all_cards = transform_prints(all_cards_vec);

    return all_cards;
}