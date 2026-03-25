#pragma once
#include <cstdint>
#include "nlohmann/json.hpp"

// ============================================================
//  Format bit positions (2 bits each within LegalityEncoding)
// ============================================================

enum class Format : uint8_t {
    standard  = 0,
    pioneer   = 2,
    modern    = 4,
    legacy    = 6,
    vintage   = 8,
    commander = 10,
    pauper    = 12,
    brawl     = 14,
    historic  = 16,
    timeless  = 18
    // 20 bits total, fits in uint32_t
};

// ============================================================
//  Legality status (2-bit values stored per format)
// ============================================================

enum class LegalityStatus : uint8_t {
    baseline   = 0b00,  // defer to LegalityEncoding::baseline
    legal      = 0b01,
    restricted = 0b10,
    illegal    = 0b11
};

// ============================================================
//  Encoding struct
// ============================================================

struct LegalityEncoding {
    uint32_t exceptions;    // 2 bits per format, 0b00 means "same as baseline"
    bool     baseline;      // true = legal by default, false = illegal by default
};

// ============================================================
//  Interface
// ============================================================

/**
 * @brief Encode a Scryfall legalities JSON object into a LegalityEncoding
 * @param legalities_obj The "legalities" field from a Scryfall card element
 * @return Populated LegalityEncoding struct
 */
LegalityEncoding encode_legalities(const nlohmann::json& legalities_obj, const bool game_changer=false);

/**
 * @brief Decode the legality status of a single format from an encoding
 * @param enc     The LegalityEncoding to query
 * @param fmt     The format to look up
 * @return LegalityStatus for that format
 */
LegalityStatus get_format_status(const LegalityEncoding& enc, Format fmt);