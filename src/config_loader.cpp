#include <filesystem>
#include <spdlog/spdlog.h>
#include <format>
#include <toml++/toml.hpp>
#include "config_loader.hpp"

std::pair<std::vector<std::string>, std::string>  ConfigLoader::getPaths(const std::string& path) {
    auto config = toml::parse_file(path);
    auto main_table = config["main"].as_table();
    if (!main_table || !main_table->contains("input"))
        throw std::runtime_error("Invalid configuration file: missing 'input' parameter.");
    std::string input_dir = (*main_table)["input"].value_or<std::string>("");
    std::string output_dir = (*main_table)["output"].value_or<std::string>("");
    std::vector<std::string> masks = getMasks(path);
    std::vector<std::string> input = getInput(input_dir, masks);
    std::string output = getOutput(output_dir);
    return {input, output};
}

std::vector<std::string>    ConfigLoader::getInput(const std::string& dir, const std::vector<std::string>& masks) {
    std::vector<std::string>    input_files;
    if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir))
        throw std::runtime_error(std::format("Invalid input directory path: {}", dir));
    for (const auto& file : std::filesystem::directory_iterator(dir)) {
        if (!file.is_regular_file() || file.path().extension() != ".csv")
            continue;
        const std::string filename = file.path().filename().string();
        if (masks.empty()) {
            input_files.emplace_back(file.path().string());
            continue;
        }        
        bool matches = false;
        for (const auto& mask : masks)
            if (filename.find(mask) != std::string::npos) {
                matches = true;
                break;
            }
        if (matches)
            input_files.emplace_back(file.path().string());
    }
    if (input_files.empty())
        spdlog::warn(std::format("No CSV files matching the criteria found in: {}", dir));
    return input_files;
}

std::string ConfigLoader::getOutput(const std::string& dir) {
    std::string output_dir = dir;
    if (output_dir.empty()) {
        output_dir = "./output";
        spdlog::info(std::format("Output directory not specified. Using default: {}", output_dir));
    }    
    if (!std::filesystem::exists(output_dir)) {
        std::filesystem::create_directories(output_dir);
        spdlog::info(std::format("Created output directory: {}", output_dir));
    }
    if (!std::filesystem::is_directory(output_dir))
        throw std::runtime_error(std::format("Output path '{}' is not a directory", output_dir));
    return output_dir;
}

std::vector<std::string> ConfigLoader::getMasks(const std::string& config_path) {
    auto    config = toml::parse_file(config_path);
    auto    main_table = config["main"].as_table();
    if (!main_table || !main_table->contains("filename_mask"))
        return {}; 
    auto    masks_array = (*main_table)["filename_mask"].as_array();
    if (!masks_array)
        return {};
    std::vector<std::string> masks;
    for (const auto& item : *masks_array) {
        if (auto str = item.value<std::string>()) {
            masks.emplace_back(*str);
        }
    }
    return masks;
}
