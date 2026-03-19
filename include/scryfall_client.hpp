#pragma once
#include <string>

   /** 
 * @brief Client side bulk data aggregator
 *
 * This is the client that will be fetching the
 * json data from scryfall's public api befor
 * parsing takes place
 */
class ScryfallClient {
public:

   /** 
 * @brief Fetch the bulk data all at once and aggregate to a big string
 * @param url to be fetched from (passed by reference)
 * @return string that is the aggregated json from the given url
 *
 * note that the string is not parsed yet but just a lump sum string
 */
    std::string fetch_bulk_data(const std::string& url);
};
