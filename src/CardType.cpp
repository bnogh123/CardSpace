#include "../include/CardType.hpp"
#include <string>

void TypeEncoding::add(CardType t) {
    value *= static_cast<uint64_t>(t);
}

bool TypeEncoding::has(CardType t) const {
    return value % static_cast<uint64_t>(t) == 0;
}

bool TypeEncoding::is_exactly(CardType t) const{
    return value == static_cast<uint64_t>(t) == 0;
}

TypeEncoding encode_type_line(const std::string& type_line){
    TypeEncoding result;    // starts with value = 1

    // 1. find the em-dash, take left side
    std::string left;
    std::size_t pos = type_line.find(" \xe2\x80\x94 ");

    if (pos != std::string::npos) {
        left        = type_line.substr(0, pos);
    } else {
        left        = type_line; // no em-dash found, card has no subtypes
    }

    // 2. split left side into words
    // 3. for each word, look up in map and call result.add()

    return result;
}
