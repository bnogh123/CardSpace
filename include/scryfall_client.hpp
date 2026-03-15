#pragma once
#include <string>

// This is the client that will be fetching the
// json data from scryfall's public api
class ScryfallClient {
public:

/*  input: url to be fetched from (passed by reference)
    output: string that is the aggregated json from the given url
    note that the string is not parsed yet but just a lump sum string   */
    std::string fetch_bulk_data(const std::string& url);
};
