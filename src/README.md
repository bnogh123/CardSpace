# `src/` — Pipeline Implementation

<!-- TODO: One short paragraph explaining that this directory contains the
     implementation of all pipeline stages and encoding modules. Readers
     looking for interface definitions and data model should refer to include/. -->

---

## Pipeline Stages

### `etl_pipeline.cpp`
<!-- TODO: Describe how this file orchestrates the full Extract → Transform → Load
     flow. What does it call and in what order? Is there any error handling or
     short-circuit logic at the top level worth noting? -->

### `scryfall_client.cpp`
<!-- TODO: Describe the extraction implementation — how bulk JSON is fetched,
     how the local oracle.json cache is checked before downloading, and how
     Scryfall's rate limit guidelines are respected. -->

### `card_parser.cpp`
<!-- TODO: Describe how raw JSON objects are deserialized into Card and
     PrintedCard structs. Note how std::optional fields are handled —
     what happens when an expected field is absent from a given card's JSON. -->

### `transformer.cpp`
<!-- TODO: Describe what transformations are applied after parsing — field
     normalization, encoding calls, or any enrichment steps before the
     card is handed to the loader. -->

### `loader.cpp`
<!-- TODO: Describe how normalized Card and PrintedCard structs are inserted
     into the SQLite warehouse. Note any batch behavior, transaction handling,
     or conflict resolution logic. -->

### `warehouse.cpp`
<!-- TODO: Describe schema management and connection handling — how the SQLite
     database is initialized, what tables are created, and whether schema
     migrations are supported. -->

---

## Encoding Modules

<!-- TODO: Brief intro noting that these modules implement the encoding schemes
     defined in the corresponding headers under include/. Readers looking for
     the rationale behind each scheme should refer to include/README.md. -->

### `CardType.cpp`
<!-- TODO: Describe the product-of-primes encoding implementation for card types.
     Note any edge cases — cards with no type, or types not present in the
     prime map. -->

### `CardColor.cpp`
<!-- TODO: Same structure as CardType — describe the encoding implementation
     and any edge cases (colorless cards, color indicator vs color identity). -->

### `CardLegality.cpp`
<!-- TODO: Describe the baseline-exception encoding implementation. Note how
     the baseline is determined, how exceptions are packed into the bitmask,
     and how the game changer special case is applied. -->

---

## Utilities

### `utils.cpp`
<!-- TODO: Describe the shared utilities implemented here. Note which pipeline
     stages depend on them and what problem each utility solves. -->

### `card_functions.cpp`
<!-- TODO: Describe what this module provides — helper functions for card data
     manipulation, shared logic across pipeline stages, or something else.
     Clarify the relationship between this file and scripts/card_functions.py
     if relevant. -->

---

## Notes
<!-- TODO: Anything that doesn't fit above — build quirks, known limitations,
     implementation tradeoffs considered and rejected, or pointers to docs/
     for deeper rationale. -->