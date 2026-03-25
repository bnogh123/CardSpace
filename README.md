# CardSpace

A C++ ETL pipeline that pulls Magic: The Gathering card data from Scryfall's
bulk data API, parses and normalizes it, and loads it into a SQLite data
warehouse for ML and analytics use. This is a portfolio project exploring
data engineering concepts in C++, with a parallel Jupyter notebook layer
for analysis.

> **Status:** Active development — pipeline stages are implemented
> incrementally. See [Project Structure](#project-structure) for current state.

## The Pipeline
```
Scryfall Bulk API
      │
      ▼
 [ Extract ]  scryfall_client   — downloads bulk JSON (~100MB), respects
                                  Scryfall's rate limit guidelines
      │
      ▼
 [ Transform ] card_parser      — filters cards via CardFilter predicates,
                                  normalizes fields, encodes types/colors
      │
      ▼
 [ Load ]      sqlite_loader    — inserts normalized Card structs into
                                  the SQLite warehouse
```

## Features

**Bulk download over live API** — CardSpace uses Scryfall's weekly bulk data
endpoint rather than individual card queries. This avoids rate limiting,
produces consistent snapshots, and is the approach Scryfall themselves
recommend for data analysis use cases.

**Two-struct data model** — Card data is split across two structs mirroring
Scryfall's own oracle/print distinction. `Card` holds oracle data — the
canonical gameplay identity of a card (rules text, types, colors, legalities)
encoded for analysis. `PrintedCard` holds print-specific data — the physical
attributes of a particular printing (set, collector number, foil treatment,
artwork). This separation keeps analytical queries over gameplay data clean
and uncluttered by the combinatorial explosion of print variants.

**Product-of-primes encoding** — Card types, colors, and color identity are
encoded as products of primes rather than bitmask flags. This makes
multi-label membership queries expressible as divisibility checks, which
is both compact and analytically interesting for downstream ML feature
engineering.

**Baseline-exception legality encoding** — Format legality is encoded in
`CardLegality` using a compact baseline-exception scheme rather than a naive
bitmask. A single `baseline` bit captures whether a card is legal in the
majority of the 10 tracked formats (Standard, Pioneer, Modern, Legacy,
Vintage, Commander, Pauper, Brawl, Historic, Timeless). Only formats that
deviate from that baseline are stored in an `exceptions` field, where each
tracked format occupies a 2-bit slot encoding one of three states: legal,
restricted, or illegal. Game Changer cards receive special handling — their
Commander status is forced to restricted regardless of the baseline. This
approach minimizes storage for cards with uniform legality profiles (e.g. a
card banned everywhere stores a single `false` baseline and no exceptions)
while still supporting precise per-format queries via bitwise extraction.

## Project Structure
```
CardSpace/
├── apps/
│   ├── main.cpp          # Entry point for the ETL pipeline
│   └── query_main.cpp    # Entry point for warehouse queries
├── src/                  # Pipeline implementation
│   ├── scryfall_client   # Bulk JSON download and caching
│   ├── card_parser       # JSON → Card struct deserialization
│   ├── transformer       # Field normalization and encoding
│   ├── loader            # Card insertion into SQLite
│   ├── warehouse         # SQLite schema and connection management
│   ├── etl_pipeline      # Orchestrates the full Extract→Transform→Load flow
│   └── CardType/Color/Legality  # Product-of-primes encoding modules
├── include/              # Headers and data model (see include/README.md)
│   ├── Card.hpp          # Oracle data — rules text, types, colors, legalities
│   ├── PrintedCard.hpp   # Print data — set, collector number, foil, art
│   └── ...
├── tests/                # Catch2 test suites (one per module)
├── notebooks/            # Jupyter analysis + Python helper scripts
├── docs/                 # Design rationale and architecture notes
├── bin/                  # Compiled binaries (after build)
└── CMakeLists.txt
```

See each directory's README for details.

## Installation

### Dependencies

| Dependency     | How it's managed         |
|----------------|--------------------------|
| libcurl        | System package (see below) |
| nlohmann/json  | CMake FetchContent       |
| fmtlib         | CMake FetchContent       |
| Catch2         | CMake FetchContent       |
| SQLite3        | System package (see below) |

Install system packages first:
```bash
# Ubuntu / Debian
sudo apt install libcurl4-openssl-dev libsqlite3-dev

# macOS (Homebrew)
brew install curl sqlite
```

### Build
```bash
git clone https://github.com/bnogh123/CardSpace.git
cd CardSpace
cmake -B build
cmake --build build
```

## Usage
```bash
# Run the full ETL pipeline
# Reads oracle.json if already cached, otherwise downloads from Scryfall
./bin/cardspace_etl

# Query the warehouse interactively
./bin/cardspace_query
```

> `oracle.json` is Scryfall's bulk card data (~100MB). On first run it is
> downloaded and cached at the project root. Subsequent runs reuse it.
## Tech Stack

| Tool           | Role                        |
|----------------|-----------------------------|
| C++17          | Core pipeline language      |
| CMake          | Build system (FetchContent) |
| libcurl        | HTTP for bulk data fetch    |
| nlohmann/json  | JSON parsing                |
| SQLite3        | Data warehouse target       |
| fmtlib         | String formatting           |
| Catch2         | Unit testing                |
| Jupyter/pandas | Notebook analysis layer     |