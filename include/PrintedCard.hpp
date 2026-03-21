#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <optional>

/**
 * @brief Represents a specific physical printing of a card.
 * 
 * Every PrintedCard must have a corresponding Card with a matching
 * o_id. The o_id field is a required foreign key — a PrintedCard
 * cannot exist without a parent oracle Card.
 */
struct PrintedCard {
    // --- identity ---
    std::string id;             // scryfall printing ID (unique per printing)
    std::string o_id;           // oracle ID — foreign key to Card::o_id

    // --- printing metadata ---
    std::string set;            // set code (e.g. "lea", "m21")
    std::string set_id;         // scryfall set UUID
    std::string set_name;       // full set name (e.g. "Limited Edition Alpha")
    std::string set_type;       // "expansion", "core", "masters", etc.
    std::string collector_number;
    std::string artist;
    std::string border_color;   // "black", "white", "borderless", etc.
    std::string frame;          // "1993", "1997", "2003", "2015", "future"
    std::string released_at;    // ISO 8601 date string (e.g. "1993-08-05")
    std::string image_status;   // "highres_scan", "lowres", "placeholder", etc.
    char rarity;                // 'c', 'u', 'r', 'm', 'b', 's'
    uint16_t layout;            // encoded via layouts lookup table

    // --- finish & format flags ---
    bool foil;
    bool nonfoil;
    bool oversized;
    bool full_art;
    bool textless;
    bool promo;
    bool reprint;
    bool digital;
    bool booster;
    bool reserved;
    bool variation;
    bool story_spotlight;
    bool highres_image;

    // --- pricing ---
    std::optional<double> usd_price;
    std::optional<double> usd_foil_price;

    // --- optional collections ---
    std::optional<std::vector<int>> multiverse_ids;
    std::optional<std::vector<std::string>> games;  // "paper", "mtgo", "arena"
    std::optional<std::vector<std::string>> finishes;

    // --- uris (optional — not always needed for analytics) ---
    std::optional<std::string> scryfall_uri;
    std::optional<std::string> rulings_uri;
    std::optional<std::string> prints_search_uri;
};