// src/warehouse.cpp

#include "warehouse.hpp"
#include <optional>
#include <string>
#include <vector>

Warehouse::Warehouse(const std::string& db_path) : loader_(db_path) {
    // Initialize the schema when the Warehouse is created
    loader_.init_schema();
}

void Warehouse::load_cards(const std::vector<TransformedCard>& cards) {
    loader_.load_cards(cards);
}

void Warehouse::load_prints(const std::vector<TransformedPrint>& prints) {
    loader_.load_prints(prints);
}

std::optional<TransformedCard> Warehouse::get_card_by_o_id(const std::string& o_id) const {
    // Implement the logic to query a card by its Oracle ID
    // This will involve using SQL queries to fetch the data from the database
    // For now, let's assume this method is implemented and returns an optional TransformedCard
    return loader_.get_card_by_o_id(o_id);
}

std::optional<TransformedPrint> Warehouse::get_print_by_id(const std::string& id) const {
    // Implement the logic to query a printed card by its Scryfall ID
    // This will involve using SQL queries to fetch the data from the database
    // For now, let's assume this method is implemented and returns an optional TransformedPrint
    return loader_.get_print_by_id(id);
}