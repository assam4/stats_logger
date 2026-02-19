#include <format>
#include <boost/program_options.hpp>
#include <spdlog/spdlog.h>
#include "toml_config_parser.hpp"
#include "parser_csv.hpp"
#include "concurrent_log_parser.hpp"
#include "logger.hpp"

static std::string get_configuration_file(int argc, char **argv) {
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("config", po::value<std::string>(), "Configuration file path")
        ("cfg", po::value<std::string>(), "Configuration file path");
    po::variables_map   vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    std::string configFile =  "default_config.toml";
    if (vm.count("config"))
        configFile = vm["config"].as<std::string>();
    else if (vm.count("cfg"))
        configFile = vm["cfg"].as<std::string>();
    return configFile;
}

void    run(const std::string& config_file) {
    spdlog::info(std::format("launch of version 1.1.0 of the program"));
    spdlog::info(std::format("processing of the {} configuration file", config_file)); 
    auto    io = TomlConfigLoader::getPaths(config_file);
    size_t threads_count = std::thread::hardware_concurrency();
    if (!threads_count)
        threads_count = 2;
    ConcurrentLogParser<data, decltype(Parser_csv::parse), decltype(get_chunks)> clp(threads_count, Parser_csv::parse, get_chunks);
    auto    result = clp.collect(io.first);
    if (!result)
            throw std::runtime_error("Something went wrong...");
    Logger  logger(result.value(), io.second);
    logger.switchLogger();
}
    
int main(int argc, char **argv) {
    try {
        auto config = get_configuration_file(argc, argv);
        run(config);
        return 0;
    }
    catch (const std::exception& e) {
        spdlog::error(std::format("Error: {}", e.what()));
        return 1;
    }
    catch (...) {
        spdlog::error("Error: Unknown bag...");
        return 1;
    }
} 




