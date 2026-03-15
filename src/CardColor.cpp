#include "../include/CardColor.hpp"
#include <string>
#include <unordered_map>
#include <vector>

static const std::unordered_map<std::string, CardColor> lookup = {
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
*/
bool ColorEncoding::has(CardColor t) const {
    return value % static_cast<uint64_t>(t) == 0;
}

/*
*/
bool ColorEncoding::is_exactly(CardColor t) const{
    return value == static_cast<uint64_t>(t);
}

/*
*/
bool ColorEncoding::is_multicolor() const{
    return !is_prime(value) && value != 1;
}

/*
*/
ColorEncoding encode_color(const std::string& text){
    ColorEncoding result;    // starts with value = 1
    std::vector<std::string> extracted;
    extracted = extract_braced(text);
    
    /*  
    word is one token at a time
    loop exits automatically when stream is exhausted
    3. for each word, look up in map and call result.add()  */
    for(std::string word : extracted) {
        auto found = lookup.find(word);

        if (found != lookup.end()) {
            result.add(found->second);
        }
    }

    return result;
}

/* Extracts the values x 
input: s
encapsulated within two brackets {x}  */
std::vector<std::string> extract_braced(const std::string& text) {
    std::vector<std::string> results;
    size_t pos = 0;

    while ((pos = text.find('{', pos)) != std::string::npos) {
        size_t end = text.find('}', pos);
        if (end == std::string::npos) break;  // malformed, no closing brace

        // extract what's between { and }
        results.push_back(text.substr(pos + 1, end - pos - 1));

        pos = end + 1;  // move past this } to search for the next {
    }

    return results;
}