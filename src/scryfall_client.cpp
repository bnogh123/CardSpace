#include "../include/scryfall_client.hpp"
#include <stdexcept>
#include <curl/curl.h>

static size_t write_callback(void* contents, size_t size, 
                             size_t nmemb, std::string* output) {
    // append the next chunk to the output string
    output->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}
                

std::string ScryfallClient::fetch_bulk_data(const std::string& url){
    // init variables
    CURL* handle = curl_easy_init();
    std::string response;
    
    // error checking for initialization
    if(!handle){
        throw std::runtime_error("failed to initialize curl handle");
    }

    curl_easy_setopt(handle, CURLOPT_URL,               url.c_str());
    curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION,    1L);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION,     write_callback);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA,         &response);
    CURLcode res = curl_easy_perform(handle);
    curl_easy_cleanup(handle);

    // error checking from curl's built in detection
    if (res != CURLE_OK) {
        throw std::runtime_error(curl_easy_strerror(res));
    }

    return response;
}