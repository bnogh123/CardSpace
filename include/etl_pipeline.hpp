#pragma once
#include <string>

class ETLPipeline {
public:
    void run(const std::string& source_url, const std::string& output_path);
};