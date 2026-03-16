#pragma once
#include <cstdint>
#include <string>
#include <vector>

// String helpers
std::string to_lower(const std::string& s);

// Extracts vector of each string x that is enclosed within
// two brackets {x} in order of occurence in the given text
std::vector<std::string> extract_braced(const std::string& text);

// Math helpers
bool is_prime(uint64_t value);