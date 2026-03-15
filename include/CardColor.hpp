#pragma once
#include "utils.hpp"
#include <cstdint>
#include <string>


// Each color is a unique prime - combine with multiplication,
// test membership with modulo
enum class CardColor : uint16_t {
    NONE            = 1,

    // Colors in WUBRG order
    WHITE           = 2,
    BLUE            = 3,
    BLACK           = 5,
    RED             = 7,
    GREEN           = 11
};

// Represents a card's full color or color identity as a product of primes
struct ColorEncoding {
    uint16_t value = 1;

    void        add(CardColor t);
    bool        has(CardColor t) const;
    bool        is_exactly(CardColor t) const;
    bool        is_multicolor() const;
    uint16_t    raw() const;
};

ColorEncoding encode_color(const std::string& text);
