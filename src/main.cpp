#include <format>
#include <boost/program_options.hpp>
#include <spdlog/spdlog.h>
#include "config_loader.hpp"
#include "parser_csv.hpp"
#include "concurrent_log_parser.hpp"
#include "logger.hpp"

std::string getFile(int argc, char **argv) {
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("config", po::value<std::string>(), "path to config file")
        ("cfg", po::value<std::string>(), "path to config file");
    po::variables_map   vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    std::string fileName = "config.toml";
    if (vm.count("config"))
        fileName = vm["config"].as<std::string>();
    else if (vm.count("cfg"))
        fileName = vm["cfg"].as<std::string>();
    return fileName;
}

void    run(const std::string& fname) {
    spdlog::info(std::format("launch of version 1.1.0 of the program"));
    spdlog::info(std::format("processing of the {} file", fname)); 
    auto    io = ConfigLoader::getPaths(fname);
    ConcurrentLogParser<data, decltype(Parser_csv::parse)> clp(4, Parser_csv::parse);
    auto    result = clp.collect(io.first);
    if (!result)
            throw std::runtime_error("Something went wrong...");
    Logger  logger(result.value(), io.second);
    logger.switchLogger();
}
    
int main(int argc, char **argv) {
    try {
        auto    fileName = getFile(argc, argv);
        run(fileName);
        return 0;
    }
    catch (const std::exception& e) {
        spdlog::error(std::format("Error: {}", e.what()));
        return 1;
    }
    catch (...) {
        spdlog::error(std::format("Error: {}", "Unknown bag..."));
        return 1;
    }
} 




