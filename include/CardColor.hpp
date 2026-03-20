#pragma once
#include "../include/nlohmann/json.hpp"
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

    /**
  * @brief Add a card color to the encoding that is not present
  * @param t Color to add to the encoding (can be present in the encoding but nothing will happen)
  */
    void        add(CardColor t);

    /**
  * @brief Return True if the given color is present in the encoding
  * @param t Color that is being checked for
  * @return bool True if the color is present
  */
    bool        has(CardColor t) const;

    /**
  * @brief Return True iff an encoding is exactly the given color
  * @param t Color that is being checked for
  * @return bool True iff the encoding is exactly the given color
  */
    bool        is_exactly(CardColor t) const;

    /**
  * @brief Return True if the color encoding is multicolor 
  * @return bool True if the encoding is not 0 or exactly 1 color 
  */
    bool        is_multicolor() const;

    /**
  * @brief Accessor for raw encoding value
  * @return unsigned 2 byte int representing the raw encoding 
  */
    uint16_t    raw() const;
};

/**
  * @brief Encode color encoding from string literal
  * @param text string containing color symbol text curled in brackets (typically oracle text)
  * @return ColorEncoding struct representing the colors present
  */
ColorEncoding encode_color(const std::string& text);

/**
  * @brief Encode color encoding from string literal
  * @param json json object containing a vector of strings, each a single char long representing a color
  * @return ColorEncoding struct representing the colors present
  */
ColorEncoding encode_color(const nlohmann::json& json);
