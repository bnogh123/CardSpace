#pragma once
#include <string>


class ScryfallClient {
public:
    std::string fetch_bulk_data(const std::string& url);
};