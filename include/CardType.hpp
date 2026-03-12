#pragma once
#include <cstdint>
#include <string>


// Each type and supertype is a unique prime - combine with multiplication,
// test membership with modulo
enum class CardType : uint64_t {
    NONE            = 1,

    // Supertypes
    BASIC           = 2,
    LEGENDARY       = 3,
    SNOW            = 5,
    WORLD           = 7,

    // Card types
    CREATURE        = 11,
    LAND            = 13,
    INSTANT         = 17,
    SORCERY         = 19,
    ARTIFACT        = 23,
    ENCHANTMENT     = 29,
    PLANESWALKER    = 31,
    BATTLE          = 37
};

// Represents a card's full type as a product of primes
struct TypeEncoding {
    uint64_t value = 1;

    void        add(CardType t);
    bool        has(CardType t) const;
    bool        is_exactly(CardType t) const;
    bool        is_multitype() const;
    uint64_t    raw() const;
};

TypeEncoding encode_type_line(const std::string& type_line);