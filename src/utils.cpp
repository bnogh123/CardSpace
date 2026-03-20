#include "../include/utils.hpp"
#include <algorithm>
#include <vector>

std::string to_lower(const std::string& s){
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
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

/* 
input: value
returns true if the value is prime and false otherwise */
bool is_prime (const uint64_t& value) {
    uint16_t cnt = 0;
    bool is_prime;

    // If number is less than/equal 
    // to 1 and number is even accept 2
    // then it is not prime
    if (value <= 1 || ((value > 2) && (value%2 == 0)))
        is_prime = false;
    else {
        if (value == 2) {
            is_prime = true;
        } else {

            // Check how many numbers divide 
            // n in the range 2 to sqrt(n)
            for (int i = 3; i * i <= value; i += 2) {
                if (value % i == 0)
                    cnt++;
            }

            // if cnt is greater than 0, 
            // then n is not prime
            if (cnt > 0)
                is_prime = false;

            // else n is prime
            else
                is_prime = true;
        }
    }

    return is_prime;
}