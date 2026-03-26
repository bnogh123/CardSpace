#include "../include/transformer.hpp"
#include <sstream>
#include <variant>

// ============================================================
//  Internal helpers
// ============================================================

// Flatten std::variant<BasePT, CDAPT, NegativePT> into three
// independently nullable warehouse columns.
static void flatten_pt(
    const std::variant<BasePT, CDAPT, NegativePT>& pt,
    std::optional<int32_t>& out_power,
    std::optional<int32_t>& out_toughness,
    std::optional<uint8_t>& out_flags)
{
    std::visit([&](auto&& v) {
        using T = std::decay_t<decltype(v)>;

        if constexpr (std::is_same_v<T, BasePT>) {
            out_power     = static_cast<int32_t>(v.power);
            out_toughness = static_cast<int32_t>(v.toughness);
            out_flags     = std::nullopt;

        } else if constexpr (std::is_same_v<T, CDAPT>) {
            out_power     = static_cast<int32_t>(v.power);
            out_toughness = static_cast<int32_t>(v.toughness);
            out_flags     = v.cda_flags;               // 0b01, 0b10, or 0b11

        } else if constexpr (std::is_same_v<T, NegativePT>) {
            out_power     = -static_cast<int32_t>(v.power);  // negate absolute value
            out_toughness = static_cast<int32_t>(v.toughness);
            out_flags     = std::nullopt;
        }
    }, pt);
}

// Serialize a string vector as pipe-delimited.
// Pipe chosen over comma because oracle text already contains commas.
static std::string pipe_join(const std::vector<std::string>& vec) {
    if (vec.empty()) return "";
    std::ostringstream oss;
    for (std::size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) oss << '|';
        oss << vec[i];
    }
    return oss.str();
}

// Serialize a vector<int> as comma-delimited (used for multiverse_ids).
static std::string comma_join(const std::vector<int>& vec) {
    if (vec.empty()) return "";
    std::ostringstream oss;
    for (std::size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) oss << ',';
        oss << vec[i];
    }
    return oss.str();
}

// Pack all PrintedCard boolean flags into a single bitmask.
static uint16_t pack_print_flags(const PrintedCard& p) {
    uint16_t flags = 0;
    if (p.foil)            flags |= FLAG_FOIL;
    if (p.nonfoil)         flags |= FLAG_NONFOIL;
    if (p.oversized)       flags |= FLAG_OVERSIZED;
    if (p.full_art)        flags |= FLAG_FULL_ART;
    if (p.textless)        flags |= FLAG_TEXTLESS;
    if (p.promo)           flags |= FLAG_PROMO;
    if (p.reprint)         flags |= FLAG_REPRINT;
    if (p.digital)         flags |= FLAG_DIGITAL;
    if (p.booster)         flags |= FLAG_BOOSTER;
    if (p.reserved)        flags |= FLAG_RESERVED;
    if (p.variation)       flags |= FLAG_VARIATION;
    if (p.story_spotlight) flags |= FLAG_STORY_SPOTLIGHT;
    if (p.highres_image)   flags |= FLAG_HIGHRES_IMAGE;
    return flags;
}

// ============================================================
//  Single-record transforms
// ============================================================

TransformedCard transform_card(const Card& card) {
    TransformedCard out;

    // --- identity & text ---
    out.o_id          = card.o_id;
    out.name          = card.name;
    out.mana_cost     = card.mana_cost;
    out.cmc           = card.cmc;
    out.type_line_raw = card.type_line_raw;
    out.oracle_text   = card.oracle_text;

    // --- encoded fields: pull raw value from each encoding struct ---
    out.type_encoded  = card.type.raw();
    out.color         = card.color.raw();
    out.color_identity = card.color_identity.raw();

    // --- legalities: split LegalityEncoding into two flat fields ---
    out.legality_exceptions = card.legalities.exceptions;
    out.legality_baseline   = card.legalities.baseline;

    // --- keywords ---
    out.keywords_str = pipe_join(card.keywords);

    // --- p/t variant ---
    if (card.pt.has_value())
        flatten_pt(card.pt.value(), out.power, out.toughness, out.cda_flags);

    // --- other optional stats ---
    out.loyalty  = card.loyalty;
    out.defense  = card.defense;

    return out;
}

TransformedPrint transform_print(const PrintedCard& print) {
    TransformedPrint out;

    // --- identity ---
    out.id   = print.id;
    out.o_id = print.o_id;

    // --- set metadata ---
    out.set              = print.set;
    out.set_id           = print.set_id;
    out.set_name         = print.set_name;
    out.set_type         = print.set_type;
    out.collector_number = print.collector_number;
    out.artist           = print.artist;
    out.border_color     = print.border_color;
    out.frame            = print.frame;
    out.released_at      = print.released_at;
    out.image_status     = print.image_status;

    // --- classification ---
    out.rarity = print.rarity;
    out.layout = print.layout;

    // --- boolean flags ---
    out.flags = pack_print_flags(print);

    // --- pricing ---
    out.usd_price      = print.usd_price;
    out.usd_foil_price = print.usd_foil_price;

    // --- optional collections ---
    if (print.multiverse_ids.has_value())
        out.multiverse_ids_str = comma_join(print.multiverse_ids.value());

    if (print.games.has_value())
        out.games_str = pipe_join(print.games.value());

    if (print.finishes.has_value())
        out.finishes_str = pipe_join(print.finishes.value());

    // --- uris ---
    out.scryfall_uri      = print.scryfall_uri.value_or("");
    out.rulings_uri       = print.rulings_uri.value_or("");
    out.prints_search_uri = print.prints_search_uri.value_or("");

    return out;
}

// ============================================================
//  Batch helpers
// ============================================================

std::vector<TransformedCard> transform_cards(const std::vector<Card>& cards) {
    std::vector<TransformedCard> out;
    out.reserve(cards.size());
    for (const auto& c : cards)
        out.push_back(transform_card(c));
    return out;
}

std::vector<TransformedPrint> transform_prints(const std::vector<PrintedCard>& prints) {
    std::vector<TransformedPrint> out;
    out.reserve(prints.size());
    for (const auto& p : prints)
        out.push_back(transform_print(p));
    return out;
}