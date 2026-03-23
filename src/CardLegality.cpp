#include <cstdint>
#include <unordered_set>
#include <unordered_map>
#include "../include/nlohmann/json.hpp"
#include "../include/CardLegality.hpp"

static constexpr int TRACKED_FORMATS = 10;

static const std::unordered_set<std::string> ignored_formats = {
    "alchemy", "explorer", "oathbreaker", "standardbrawl", "paupercommander", "predh", "premodern"
};

static const std::unordered_map<std::string, Format> format_map = {
    {"standard",  Format::standard},
    {"pioneer",   Format::pioneer},
    {"modern",    Format::modern},
    {"legacy",    Format::legacy},
    {"vintage",   Format::vintage},
    {"commander", Format::commander},
    {"pauper",    Format::pauper},
    {"brawl",     Format::brawl},
    {"historic",  Format::historic},
    {"timeless",  Format::timeless}
};

LegalityEncoding encode_legalities(const nlohmann::json &legalities_obj, const bool game_changer) {
    LegalityEncoding enc{0, false};
    
    // determine the baseline
    int legal_count = 0;
    int total       = 0;
    for (const auto& [format, status] : legalities_obj.items()) {
        if (ignored_formats.count(format)) continue;

        std::string s = status.get<std::string>();
        if (s == "legal") legal_count++;
    }
    enc.baseline = (legal_count >= (TRACKED_FORMATS / 2));

    // encode the exceptions
    for (const auto& [format, status] : legalities_obj.items()) {
        std::string s = status.get<std::string>();
        LegalityStatus lst;

        if      (s == "restricted")                 lst = LegalityStatus::restricted;
        else if (s == "legal")                      lst = LegalityStatus::legal;
        else /*(s == "banned || s == "not_legal")*/ lst = LegalityStatus::illegal;

        // only store if not baseline 
        bool is_baseline = enc.baseline ? (lst == LegalityStatus::legal)
                                        : (lst == LegalityStatus::illegal);

        if (!is_baseline) {
            auto found = format_map.find(format);
            if (found != format_map.end()) {
                uint8_t shift   = static_cast<uint8_t>(found->second);
                uint8_t bits    = static_cast<uint8_t>(lst);
                enc.exceptions |= (bits << shift);  
            }
        }
    }

    // force commander to restricted for game changer cards
    if (game_changer) {
        auto found = format_map.find("commander");
        if (found != format_map.end()) {
            uint8_t shift = static_cast<uint8_t>(found->second);
            // clear existing 2 bits first, then set restricted
            enc.exceptions &= ~(0b11 << shift);
            enc.exceptions |= (static_cast<uint8_t>(LegalityStatus::restricted) << shift);
        }
    }

    return enc;
}

LegalityStatus get_format_status(const LegalityEncoding& enc, Format fmt) {
    uint8_t bits = (enc.exceptions >> static_cast<uint8_t>(fmt)) & 0b11;
    LegalityStatus status;

    if (bits == 0b00) {
        if(enc.baseline){
            status = LegalityStatus::legal;
        } else {
            status = LegalityStatus::illegal;
        }
    } else {
        status = static_cast<LegalityStatus>(bits);
    }

    return status;
}