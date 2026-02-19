#include <sstream>
#include <fstream>
#include <algorithm>
#include <exception>
#include <format>
#include <spdlog/spdlog.h>
#include "parser_csv.hpp"

static constexpr std::string_view   level_header = "receive_ts;exchange_ts;price;quantity;side;rebuild";
static constexpr std::string_view   trade_header = "receive_ts;exchange_ts;price;quantity;side";

size_t  Parser_csv::get_endIndex(const std::string &line, size_t start) {
    auto end = line.find_first_of(";", start);
    if (end == std::string::npos)
        throw std::runtime_error("Error! Missing Separator");
    return end;
}

uint64_t    Parser_csv::get_ull(const std::string& line, size_t& start, size_t end) {
    auto result = std::stoull(line.substr(start, end - start));
    start = end + 1;
    return result;
};

double  Parser_csv::get_double(const std::string& line, size_t& start, size_t end) {
    auto result = std::stod(line.substr(start, end - start));
    start = end + 1;
    return result;
}

std::string Parser_csv::get_side(const std::string& line, size_t& start) {
        auto end = line.find_first_of(";", start);
        std::string side = (end == std::string::npos) ? line.substr(start)
                                                            : line.substr(start, end - start);
        if (side != "ask" && side != "bid")
            throw std::runtime_error("Invalid value for side: " + side
                                        + "when expected 'ask' or 'bid'");
        start = end;
        return side;
}

int  Parser_csv::get_rebuild(const std::string& line, size_t& start) {
    auto  result = std::stoi(line.substr(start + 1));
    if (result != 1 && result != 0)
        throw std::runtime_error("Invalid value for rebuild: " + std::to_string(result)
                                    + "when expected '1' or '0'");
    return result;
}

std::vector<data>   Parser_csv::parse(const std::string& input) {
    std::vector<data>   logs;
    std::istringstream   is(input);
    std::string line;
        
    while (std::getline(is, line)) {
        if (line.empty()) continue;
        data    current;
        size_t    start{0};
        try {
            current.receive_ts = get_ull(line, start, get_endIndex(line, start));
            current.exchange_ts = get_ull(line, start, get_endIndex(line, start));
            current.price = get_double(line, start, get_endIndex(line, start));
            current.quantity = get_double(line, start, get_endIndex(line, start));
            current.side = get_side(line, start);
            if (start != std::string::npos)
                current.rebuild = get_rebuild(line, start);              
            logs.push_back(current);
        }
        catch (std::exception& e) {
            spdlog::debug(std::format("Parse error: {}", e.what()));
        }
        catch (...) {
            spdlog::debug(std::format("Parse error: Unknown"));
        }
    }
    std::sort(logs.begin(), logs.end());
    return logs;
}

std::expected<std::vector<std::string>, bool> get_chunks(const std::string& file) {
    std::ifstream is(file);
            if (!is.is_open()) {
                spdlog::warn(std::format("Failed to open: {}", file));
                return std::unexpected<bool>(false);;
            }
    std::vector<std::string> chunks;
    std::string header;
    std::getline(is, header);
    if (header != level_header && header != trade_header) {
        spdlog::warn(std::format("Invalid CSV header in file '{}': '{}'\n", file , header));
        return std::unexpected<bool>(false);;
    }
    char    buffer[4096];
    while (is.read(buffer, 4096) || is.gcount() > 0) {
        std::string result;
        if (is.gcount() == 4096 && buffer[is.gcount() - 1] != '\n') {
            std::string helper;
            auto    rd = is.gcount();
            std::getline(is, helper);
            result = std::string(buffer, rd) + helper;
        }
        else
            result = std::string(buffer, is.gcount());
        chunks.push_back(result);
    }
    return chunks;
}