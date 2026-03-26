#pragma once
#include "Card.hpp"
#include "PrintedCard.hpp"
#include <optional>
#include <string>
#include <vector>

// ============================================================
//  TransformedCard
//  Warehouse-ready flattening of Card. Every std::optional and
//  std::variant is resolved into a directly bindable primitive.
// ============================================================
struct TransformedCard {
    // --- identity ---
    std::string o_id;
    std::string name;

    // --- mana / cost ---
    std::string mana_cost;
    uint16_t    cmc = 0;

    // --- type ---
    std::string type_line_raw;
    uint64_t    type_encoded = 0;   // TypeEncoding::raw()

    // --- text ---
    std::string oracle_text;

    // --- color (ColorEncoding::raw() → uint16_t stored as INTEGER) ---
    uint16_t color          = 0;
    uint16_t color_identity = 0;

    // --- legalities ---
    // Stored as two fields so the warehouse can reconstruct per-format
    // status without needing C++ structs.
    uint32_t legality_exceptions = 0;
    bool     legality_baseline   = false;

    // --- keywords (pipe-delimited: "Flying|Trample|Haste") ---
    std::string keywords_str;

    // --- p/t (variant flattened) ---
    // NegativePT  → power is stored as a negative int32
    // CDAPT       → cda_flags carries the 0b01/0b10/0b11 mask
    // BasePT      → cda_flags is nullopt
    // No PT card  → all three are nullopt
    std::optional<int32_t>  power;
    std::optional<int32_t>  toughness;
    std::optional<uint8_t>  cda_flags;

    // --- other optional stats ---
    std::optional<uint16_t> loyalty;
    std::optional<uint16_t> defense;
};

// ============================================================
//  TransformedPrint
//  Warehouse-ready flattening of PrintedCard.
// ============================================================

// Boolean printing flags packed into a single uint16_t bitmask.
enum PrintFlagBit : uint16_t {
    FLAG_FOIL            = 1 << 0,
    FLAG_NONFOIL         = 1 << 1,
    FLAG_OVERSIZED       = 1 << 2,
    FLAG_FULL_ART        = 1 << 3,
    FLAG_TEXTLESS        = 1 << 4,
    FLAG_PROMO           = 1 << 5,
    FLAG_REPRINT         = 1 << 6,
    FLAG_DIGITAL         = 1 << 7,
    FLAG_BOOSTER         = 1 << 8,
    FLAG_RESERVED        = 1 << 9,
    FLAG_VARIATION       = 1 << 10,
    FLAG_STORY_SPOTLIGHT = 1 << 11,
    FLAG_HIGHRES_IMAGE   = 1 << 12,
};

struct TransformedPrint {
    // --- identity ---
    std::string id;
    std::string o_id;

    // --- set metadata ---
    std::string set;
    std::string set_id;
    std::string set_name;
    std::string set_type;
    std::string collector_number;
    std::string artist;
    std::string border_color;
    std::string frame;
    std::string released_at;
    std::string image_status;

    // --- classification ---
    char     rarity = '\0';
    uint16_t layout = 0;

    // --- flags bitmask (see PrintFlagBit) ---
    uint16_t flags = 0;

    // --- pricing ---
    std::optional<double> usd_price;
    std::optional<double> usd_foil_price;

    // --- optional collections serialized for storage ---
    // multiverse_ids → comma-delimited integers  e.g. "12,345,6789"
    // games          → pipe-delimited strings    e.g. "paper|mtgo|arena"
    // finishes       → pipe-delimited strings    e.g. "nonfoil|foil"
    std::string multiverse_ids_str;
    std::string games_str;
    std::string finishes_str;

    // --- uris (empty string when absent) ---
    std::string scryfall_uri;
    std::string rulings_uri;
    std::string prints_search_uri;
};

// ============================================================
//  Public API
// ============================================================

TransformedCard  transform_card(const Card& card);
TransformedPrint transform_print(const PrintedCard& print);

std::vector<TransformedCard>  transform_cards(const std::vector<Card>& cards);
std::vector<TransformedPrint> transform_prints(const std::vector<PrintedCard>& prints);