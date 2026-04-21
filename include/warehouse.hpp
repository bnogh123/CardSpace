#pragma once
#include "loader.hpp"
#include <string>

class Warehouse {
public:
    explicit Warehouse(const std::string& db_path);

    /**
     * @brief Load cards into the warehouse.
     * @param cards Vector of TransformedCard objects to be loaded.
     */
    void load_cards(const std::vector<TransformedCard>& cards);

    /**
     * @brief Load prints into the warehouse.
     * @param prints Vector of TransformedPrint objects to be loaded.
     */
    void load_prints(const std::vector<TransformedPrint>& prints);

    /**
     * @brief Query a card by its Oracle ID.
     * @param o_id Oracle ID of the card to query.
     * @return Optional TransformedCard object if found, nullopt otherwise.
     */
    std::optional<TransformedCard> get_card_by_o_id(const std::string& o_id) const;

    /**
     * @brief Query a printed card by its Scryfall ID.
     * @param id Scryfall ID of the printed card to query.
     * @return Optional TransformedPrint object if found, nullopt otherwise.
     */
    std::optional<TransformedPrint> get_print_by_id(const std::string& id) const;

private:
    Loader loader_;
};