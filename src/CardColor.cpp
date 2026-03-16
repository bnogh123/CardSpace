#include "../include/CardColor.hpp"
#include "../include/utils.hpp"
#include <string>
#include <unordered_map>
#include <vector>

static const std::unordered_map<std::string, CardColor> colors = {
    // Colors
    {"w",       CardColor::WHITE          },
    {"u",       CardColor::BLUE           },
    {"b",       CardColor::BLACK          },
    {"r",       CardColor::RED            },
    {"g",       CardColor::GREEN          }
};

/* Adds a color to a color encoding
input: CardColor t
static casts the color value t to the 
relevant prime number and multiplies
the color encoding to "add" that color onto it */
void ColorEncoding::add(CardColor t) {
    if (!has(t)){
        value *= static_cast<uint64_t>(t);
    }
}

/* input: CardColor t
returns true if the colorencoding
has that color encoded within */
bool ColorEncoding::has(CardColor t) const {
    return value % static_cast<uint64_t>(t) == 0;
}

/* input: CardColor t
returns true if the colorencoding
is exactly and only that color */
bool ColorEncoding::is_exactly(CardColor t) const{
    return value == static_cast<uint64_t>(t);
}

/* input: cardColor t
returns true if the colorencoding
is multicolored (if it is not prime)*/
bool ColorEncoding::is_multicolor() const{
    return !is_prime(value) && value != 1;
}

/* input: string text
returns the relevant applicable colorencoding*/
ColorEncoding encode_color(const std::string& color_text){
    ColorEncoding result;    // starts with value = 1
    std::vector<std::string> extracted;
    extracted = extract_braced(color_text);
      
/*  word is one token at a time
    loop exits automatically when vector is exhausted
    3. for each word, look up in map and call result.add()  */
    for(std::string word : extracted) {
        auto found = colors.find(to_lower(word));

        if (found != colors.end()) {
            result.add(found->second);
        }
    }

    return result;
}