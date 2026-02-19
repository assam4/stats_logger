#pragma once

#include <utility>
#include <vector>
#include <string>

/**
*   @class  TomlConfigLoader
*   @brief  utility class for reading .toml configuration
*   @details returns std::pair<input, output>, where input is std::vector of input(.log) files
*          , until output is a directory path.
*/

class   TomlConfigLoader {
    public:
        TomlConfigLoader() = delete;
        static  std::pair<std::vector<std::string>, std::string>    getPaths(const std::string&);
    private:
        static std::vector<std::string> getInput(const std::string&, const std::vector<std::string>&);
        static std::string  getOutput(const std::string&);
        static std::vector<std::string> getMasks(const std::string&);
};

