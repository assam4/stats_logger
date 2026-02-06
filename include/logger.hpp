#pragma once

#include "parser_csv.hpp"

/**
*   @class  Logger
*   @brief  main class for interacting with users
*   @details creates a log file for a specific parameter selected by the user
 */

class   Logger {
    public:
        Logger(const std::vector<data>& input, const std::string& path)
                : m_data(input), output_path(path) {}
        void    switchLogger() const;
    private:
        void    getPriceMedian() const;
        void    getPriceMin() const;
        void    getPriceMax() const;
    private:
        std::vector<data>   m_data;
        const std::string   output_path;
};
        