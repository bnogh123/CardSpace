#include "../include/card_parser.hpp"
#include "../include/nlohmann/json.hpp"
#include <string>
#include <vector>
#include <unordered_map>

static const std::unordered_map<std::string, char> rarities = {
    // Rarities
    {"bonus",           'b'},
    {"common",          'c'},
    {"mythic",          'm'},
    {"rare",            'r'},
    {"special",         's'},
    {"uncommon",        'u'}
};

static const std::unordered_map<std::string, uint16_t> layouts = {
    {"adventure",       6},
    {"case",            12},
    {"class",           9},
    {"flip",            1},
    {"leveler",         3},
    {"meld",            10},
    {"modal_dfc",       8},
    {"mutate",          7},
    {"normal",          0},
    {"prototype",       11},
    {"saga",            5},
    {"split",           2},
    {"transform",       4}

};

static char parse_rarity(const std::string& rarity){
    auto found = rarities.find(rarity);
    if (found != rarities.end()) {
        return found->second;
    } else {
        return '\0';
    }
}

static bool is_simple_card(std::string* validation, size_t size) {

}

/*
*/
std::vector<Card> parse_cards(const std::string& raw_json) {
    auto data = nlohmann::json::parse(raw_json);
    std::vector<Card> result;

    // parse entire array all at once
    for (const auto& element : data) {

        // initialize new card
        Card card;

        // parse immediately consistent options of element
        // (we assume every card has these values)
        card.id = element["id"].get<std::string>();                     //  id
        card.o_id = element["oracle_id"].get<std::string>();            //  oracle_id
        card.cmc = element["cmc"].get<std::uint16_t>();                 //  cmc
        card.type_line_raw = element["type_line"].get<std::string>();   //  type_line
        card.type = encode_type_line(card.type_line_raw);
        card.rarity = parse_rarity(element["rarity"].get<std::string>()); //  rarity
        card.color = encode_color(element["mana"]);

        //   "artist"
        //   "booster"
        //   "border_color"
        //   "collector_number"
        //   "color_identity"
        //   "digital"
        //   "finishes"
        //   "foil"
        //   "frame"
        //   "full_art"
        //   "game_changer"
        //   "games"
        //   "highres_image"
        //   "id"
        //   "image_status"
        //   "keywords"
        //   "layout"
        //   "legalities"
        //   "multiverse_ids"
        
        //   "nonfoil"
        //   "object"
        
        //   "oversized"
        //   "prices"
        //   "prints_search_uri"
        //   "promo"
        //   "related_uris"
        //   "released_at"
        //   "reprint"
        //   "reserved"
        //   "rulings_uri"
        //   "scryfall_set_uri"
        //   "scryfall_uri"
        //   "set"
        //   "set_id"
        //   "set_name"
        //   "set_search_uri"
        //   "set_type"
        //   "set_uri"
        //   "story_spotlight"
        //   "textless"
        
        //   "uri"
        //   "variation"


    }

    return result;
}