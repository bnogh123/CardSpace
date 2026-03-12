#include "../include/utils.hpp"
#include <algorithm>

std::string to_lower(const std::string& s){
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// takes a uint64 and returns true if
// the value is prime and false otherwise
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