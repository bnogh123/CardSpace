#pragma once
#include "transformer.hpp"
#include <sqlite3.h>
#include <stdexcept>
#include <string>
#include <vector>

// ============================================================
//  LoaderError
//  Thrown on any unrecoverable SQLite error.
// ============================================================
class LoaderError : public std::runtime_error {
public:
    explicit LoaderError(const std::string& msg) : std::runtime_error(msg) {}
};

// ============================================================
//  Loader
//  Owns the SQLite connection and all prepared statements.
//  Call init_schema() once before any load_* call.
//
//  Typical usage:
//      Loader db("cardspace.db");
//      db.init_schema();
//      db.load_cards(transformed_cards);    // wraps in one transaction
//      db.load_prints(transformed_prints);  // wraps in one transaction
// ============================================================
class Loader {
public:
    explicit Loader(const std::string& db_path);
    ~Loader();

    // Not copyable — owns raw sqlite3* resources
    Loader(const Loader&)            = delete;
    Loader& operator=(const Loader&) = delete;

    // CREATE TABLE IF NOT EXISTS for oracle_cards and printed_cards,
    // plus supporting indexes. Safe to call repeatedly (idempotent).
    void init_schema();

    // Single-row upserts (INSERT OR REPLACE).
    void load_card(const TransformedCard& card);
    void load_print(const TransformedPrint& print);

    // Batch upserts wrapped in a single BEGIN/COMMIT transaction.
    // On any failure the transaction is rolled back and the exception
    // is re-thrown.
    void load_cards(const std::vector<TransformedCard>& cards);
    void load_prints(const std::vector<TransformedPrint>& prints);

private:
    sqlite3*      db_          = nullptr;
    sqlite3_stmt* stmt_card_   = nullptr;
    sqlite3_stmt* stmt_print_  = nullptr;

    void prepare_statements();
    void finalize_statements();

    void begin_transaction();
    void commit_transaction();
    void rollback_transaction();

    // Bind all fields of one record to the corresponding prepared
    // statement, then sqlite3_step() it.
    void bind_and_step_card(const TransformedCard& card);
    void bind_and_step_print(const TransformedPrint& print);

    // Throws LoaderError with sqlite3_errmsg() appended to context.
    [[noreturn]] void throw_sqlite_error(const std::string& context) const;
};