#pragma once
#include <cstdint>
#include <string>
#include <vector>

/**
  * @brief Return lowercase version of passed string
  * @param s String to be replicated but lowercase (passed by reference)
  * @return String lowercase copy of passed string s
  */
std::string to_lower(const std::string& s);

/**
  * @brief Extracts a list of text inside of each set of braced brackets
  * @param text String to check for braced brackets (passed by reference)
  * @return Vector of strings enclosed within braced brackets (in order of occurence)
  */
std::vector<std::string> extract_braced(const std::string& text);

/**
  * @brief Return True iff a value is a prime number
  * @param value 8 byte unsigned int to check if is prime
  * @return bool True iff the number is prime
  */
bool is_prime(uint64_t value);