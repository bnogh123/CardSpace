# `include/` — Data Model & Interfaces

This directory defines the core data model of (Card.hpp and PrintedCard.hpp) and 
pipeline interfaces. Every .cpp in src/ implements against these headers.

---

## Data Model

### `Card.hpp`

The Card struct represents a conceptual card; details about how the card is played
and legalities of the card, without any actual specific details that are necessary
to actually own the card. It forms the basis of every card as it is considered for
deckbuilding minus the pricing. It is the gameplay identity of a card; when in game,
the layout, set, cost, etc. don't matter, so it only makes sense to reduce those
details to keep the data lean.

The optional fields (loyalty, pt, defense) are optional because not every card has
those details, and for loyalty and defense, most cards don't actually ever have those
details. Cards in play can often be made a creature due to another spell or staic
effect but to my knowledge there is no effect that makes a card into a planeswalker
or a battle.

### `PrintedCard.hpp`

The PrintedCard struct actually doesn't fully represent a printed card, which might
at first sound a bit misleading. Imagine that every Card struct is linked to at least
one PrintedCard struct within the data warehouse. Every PrintedCard is unique; there
is only one PrintedCard with those exact details. However to repeat the Card details
for each PrintedCard added a lot of bulk that was not considered necessary.

---

## Encoding Schemes

### `CardType.hpp` — Product-of-primes encoding

Product of primes encoding allows for the many card types to be recorded without
adding an exceptional amount of unnecessary data. Order of Card types does not matter
and so by using a product of prime encoding system, it allows us to keep track of 
what card types a card is through simple usage of divisibility. On top of that, we
can simply isolate all the single card type cards through identifying the prime 
numbers used. The following enum is used to represent the card types as they are
used and reflected by the prime numbers. Notably, subtypes are not recorded as they
would be far too much to record using a simple encoding like this.

```
enum class CardType : uint64_t {
    NONE            = 1,

    // Supertypes
    BASIC           = 2,
    LEGENDARY       = 3,
    SNOW            = 5,
    WORLD           = 7,

    // Card types
    CREATURE        = 11,
    LAND            = 13,
    INSTANT         = 17,
    SORCERY         = 19,
    ARTIFACT        = 23,
    ENCHANTMENT     = 29,
    PLANESWALKER    = 31,
    BATTLE          = 37
};
```

The four functions callable on a TypeEncoding are all meant to be incredibly simple to 
implement. add, has, and is_exactly all just take a CardType, however you can always
cast a uint64_t into the relevant CardType which allows for simple int manipulation
for query purposes. They all perform incredibly well seeing as how they don't have
any string parsing under the hood and are simple int operations. is_multitype() is
slightly more complicated, where it checks if the encoded value is not prime,
Finally we have raw() which just returns the raw unsigned int that the encoding 
really is under the hood.

### `CardColor.hpp` — Product-of-primes encoding

Product of primes encoding allows for the many card colors to be recorded without
adding an exceptional amount of unnecessary data. Order of colors does not matter
and so by using a product of prime encoding system, it allows us to keep track of 
what colors a card is through simple usage of divisibility. On top of that, we
can simply isolate all the monocolor cards through identifying the prime 
numbers used. The following enum is used to represent the colors as they are
used and reflected by the prime numbers.

```
enum class CardColor : uint16_t {
    NONE            = 1,

    // Colors in WUBRG order
    WHITE           = 2,
    BLUE            = 3,
    BLACK           = 5,
    RED             = 7,
    GREEN           = 11
};
```

The four functions callable on a ColorEncoding are all meant to be incredibly simple to 
implement. add, has, and is_exactly all just take a CardColor, however you can always
cast a uint64_t into the relevant CardColor which allows for simple int manipulation
for query purposes. They all perform incredibly well seeing as how they don't have
any string manip under the hood and are simple int operations. is_multicolor() is
slightly more complicated, where it checks if the encoded value is not prime,
Finally we have raw() which just returns the raw unsigned int that the encoding 
really is under the hood.



### `CardLegality.hpp` — Baseline-exception encoding

The Legality encoding is stored as 20 bits that can be encoded within a 32 bit int
using bit shifting where each format legality has 2 bits. To allow for more simplicity
in parsing, there's three statuses (legal, restricted, illegal) with a fourth baseline
status that allows for each encoding to actually only encode the two less common statuses
and leave the rest as 0b00 so that when parsing, we can see simply what the "exceptions"
to the baseline status is. 

The following enum represents how many bitshifts each format is within the 32 bit uint
```
enum class Format : uint8_t {
    standard  = 0,
    pioneer   = 2,
    modern    = 4,
    legacy    = 6,
    vintage   = 8,
    commander = 10,
    pauper    = 12,
    brawl     = 14,
    historic  = 16,
    timeless  = 18
    // 20 bits total, fits in uint32_t
};
```

For example, if we peeked at the first two bits of an encoding and see 0b00, we know that
the standard legality is whatever the baseline is, which itself is just a simple
bool. Now while there are three statuses, the likelihood that the baseline status wold be
restricted is not possible because there are only two formats which use restricted 
(vintage for actual restricted cards, and then commander game changers are represented 
as restricted legality). So we can safely state that the baseline legality status will
allways be either legal or illegal. The reason that there is not a separate status for
game changers is because it would be a waste of a status category given that it is simply
a form of restriction on the card anyways.

The encode_legalities function takes in a bool that represents whether a card is a game
changer. It is processed after the card is decided to be legal in commander, which is a
small inefficiency, given that if a card is a game changer we can assume it is legal in
commander, but realistically this is a very subtle issue that won't cause crazy 
implementation issues down the line unless the commander rules change drastically.

---

## Pipeline Interfaces

### `etl_pipeline.hpp`
<!-- TODO: Describe what this header exposes — the top-level orchestration
     interface that drives the full Extract → Transform → Load flow. -->

### `scryfall_client.hpp`
<!-- TODO: Describe the interface for the extraction stage — what it exposes
     for fetching and caching bulk JSON from Scryfall. -->

### `card_parser.hpp`
<!-- TODO: Describe the interface for JSON → Card/PrintedCard deserialization. -->

### `transformer.hpp`
<!-- TODO: Describe what transformations this interface exposes and at what
     stage of the pipeline they are applied. -->

### `loader.hpp`
<!-- TODO: Describe the interface for inserting normalized structs into SQLite. -->

### `warehouse.hpp`
<!-- TODO: Describe what this header exposes — schema management, connection
     handling, or query interfaces against the SQLite warehouse. -->

---

## Utilities

### `utils.hpp`
<!-- TODO: List the shared utilities this header provides and note which
     modules depend on them. -->

---

## Notes
<!-- TODO: Anything that doesn't fit above — known limitations, design
     tradeoffs you considered and rejected, or pointers to docs/ for
     deeper rationale. -->