#pragma once
#include <string>
#include <vector>
#include "CardType.hpp"
using namespace std;

struct Card {
    string id, name, mana_cost, type_line_raw, oracle_text, 
           rarity; 
    double usd_price;
    vector<string> color;
    vector<string> color_identity;
    vector<string> legalities;
};