#pragma once
#include "utils.hpp"
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

    /**
  * @brief Add a card type to the encoding that is not present
  * @param t Type to add to the encoding (can be present in the encoding but nothing will happen)
  */
    void        add(CardType t);

    /**
  * @brief Return True if the given type is present in the encoding
  * @param t Type that is being checked for
  * @return bool True if the type is present
  */
    bool        has(CardType t) const;

    /**
  * @brief Return True iff an encoding is exactly the given type
  * @param t Type that is being checked for
  * @return bool True iff the encoding is exactly the given type
  */
    bool        is_exactly(CardType t) const;

    /**
  * @brief Return True if the type encoding is multitype 
  * @return bool True if the encoding is not 0 or exactly 1 type 
  */
    bool        is_multitype() const;

    /**
  * @brief Accessor for raw encoding value
  * @return unsigned 2 byte int representing the raw encoding 
  */
    uint64_t    raw() const;
};

TypeEncoding encode_type_line(const std::string& type_line);
