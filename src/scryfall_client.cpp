#include "../include/scryfall_client.hpp"
#include <stdexcept>
#include <curl/curl.h>

/*  Write function for the curl handle; helper function for curl
input:  pointer to contents, size_t size of each member, 
        size_t number of members, string pointer to output of write
output: size_t that is the total bytes processed   */
static size_t write_callback(void* contents, size_t size, 
                             size_t nmemb, std::string* output) {
    // append the next chunk to the output string
    output->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}
                
/*  Fetches data chunk by chunk using curl
input: url of datat passed by reference as a string
outpt: string of aggregated json data (not parsed into json form)

This function uses curl easy options along with error checking
at the beginning of the init as well as at the end to make sure
there are no curl errors*/
std::string ScryfallClient::fetch_bulk_data(const std::string& url){
    // Init variables
    CURL* handle = curl_easy_init();
    std::string response;
    
    // Error checking for initialization
    if(!handle){
        throw std::runtime_error("failed to initialize curl handle");
    }

    // Set the URL
    curl_easy_setopt(handle, CURLOPT_URL,               url.c_str());

    // Follow redirects
    curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION,    1L);

    // Set write callback function
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION,     write_callback);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA,         &response);

    // Add User-Agent header
    curl_easy_setopt(handle, CURLOPT_USERAGENT, "CardSpace/0.1");

    // Add Accept header
    curl_easy_setopt(handle, CURLOPT_HTTPHEADER, 
        curl_slist_append(nullptr, "Accept: */*"));
    
    CURLcode res = curl_easy_perform(handle);
    curl_easy_cleanup(handle);

    // error checking from curl's built in detection
    if (res != CURLE_OK) {
        throw std::runtime_error(curl_easy_strerror(res));
    }

    return response;
}