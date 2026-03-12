#include "../include/CardType.hpp"
#include <string>
#include <unordered_map>
#include <sstream>

static const std::unordered_map<std::string, CardType> lookup = {
    // Supertypes
    {"basic",           CardType::BASIC           },
    {"legendary",       CardType::LEGENDARY       },
    {"snow",            CardType::SNOW            },
    {"world",           CardType::WORLD           },

    // Card types
    {"creature",        CardType::CREATURE        },
    {"land",            CardType::LAND            },
    {"instant",         CardType::INSTANT         },
    {"sorcery",         CardType::SORCERY         },
    {"artifact",        CardType::ARTIFACT        },
    {"enchantment",     CardType::ENCHANTMENT     },
    {"planeswalker",    CardType::PLANESWALKER    },
    {"battle",          CardType::BATTLE          }
};

// adds a type to a type encoding
// static casts the type value t to the 
// relevant prime number and multiplies
// the type encoding to "add" that type onto it
void TypeEncoding::add(CardType t) {
    if (!has(t)){
        value *= static_cast<uint64_t>(t);
    }
}

// 
bool TypeEncoding::has(CardType t) const {
    return value % static_cast<uint64_t>(t) == 0;
}

bool TypeEncoding::is_exactly(CardType t) const{
    return value == static_cast<uint64_t>(t);
}

bool TypeEncoding::is_multitype() const{
    return !is_prime(value) && value != 1;
}

TypeEncoding encode_type_line(const std::string& type_line){
    TypeEncoding result;    // starts with value = 1

    // 1. find the em-dash, take left side
    std::string left = "";
    std::size_t pos = type_line.find(" \xe2\x80\x94 ");

    if (pos != std::string::npos) {
        left        = type_line.substr(0, pos);
    } else {
        left        = type_line; // no em-dash found, card has no subtypes
    }

    // 2. split left side into words
    std::istringstream ss(to_lower(left));
    std::string word;

    // word is one token at a time
    // loop exits automatically when stream is exhausted
    // 3. for each word, look up in map and call result.add()
    while (ss >> word) {
        auto found = lookup.find(word);

        if (found != lookup.end()) {
            result.add(found->second);
        }
    }

    return result;
}