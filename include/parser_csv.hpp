#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <expected>


/**
 *  @struct  data
 *  @brief   Structure for storing CSV log entry data.
 *  @details Contains trading data with timestamps, price, quantity, and side information.
 *           Comparison operators use receive_ts as the comparison key for sorting.
 */

struct data{
    uint64_t    receive_ts;
    uint64_t    exchange_ts;
    double      price;
    double      quantity;
    std::string side;
    int rebuild = 0;
    [[nodiscard]] bool  operator<(const data& oth) const noexcept { return receive_ts < oth.receive_ts;}
    [[nodiscard]] bool  operator>(const data& oth) const noexcept { return receive_ts > oth.receive_ts;}
    [[nodiscard]] bool  operator==(const data& oth) const noexcept { return receive_ts == oth.receive_ts;}
    [[nodiscard]] bool  operator!=(const data& oth) const noexcept { return receive_ts != oth.receive_ts;}
};

/**
 *  @class   Parser_csv
 *  @brief   Utility class for parsing CSV log data.
 *  @details Parses CSV-formatted strings and returns a sorted vector of data entries.
 *           All methods are static; instantiation is not allowed.
 */

class Parser_csv {
    public:
        Parser_csv() = delete;
        virtual ~Parser_csv() = delete;
        static std::vector<data>   parse(const std::string& input);
    private:
        static uint64_t get_ull(const std::string&, size_t&, size_t);
        static double   get_double(const std::string&, size_t&, size_t);
        static size_t   get_endIndex(const std::string& , size_t);
        static std::string  get_side(const std::string&, size_t&);
        static int  get_rebuild(const std::string&, size_t&);
};


std::expected<std::vector<std::string>, bool> get_chunks(const std::string&);
