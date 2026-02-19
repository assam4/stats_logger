#pragma once

#include <filesystem>
#include <fstream>
#include <concepts>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <expected>
#include "parser_csv.hpp"

/**
 *  @class ConcurrentLogParser
 *  @brief Thread pool for concurrent CSV parsing of large files.
 *  @details Chunks files into a task queue where worker threads retrieve and parse entries
 *           concurrently, aggregating results into a sorted output vector.
 */


template <typename Data, typename ParserFunc, typename  ChunkGetterFunc>
requires requires(ParserFunc f1, ChunkGetterFunc f2, std::string s) {
    { f1(s) } -> std::same_as<std::vector<Data>>;
    { f2(s) } -> std::same_as<std::expected<std::vector<std::string>, bool>>;
}
class ConcurrentLogParser {
    public:
        ConcurrentLogParser(size_t n, ParserFunc f1, ChunkGetterFunc f2) {
            workers.reserve(n);
            for (size_t i = 0; i < n; ++i)
                workers.emplace_back([this, f1](std::stop_token st){worker_loop(st, f1);});
            chunk_getter = f2;
        }

        ~ConcurrentLogParser() = default;
        ConcurrentLogParser(const ConcurrentLogParser &) = delete;
        ConcurrentLogParser &operator=(const ConcurrentLogParser &) = delete;

        std::expected<std::vector<Data>, bool>    collect(std::vector<std::string> files) {
            taskGenerate(files);
            while (true) {
                {
                    std::lock_guard<std::mutex> lock(tasks_mutex);
                    if (tasks.empty())
                        break ;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            for (auto& w : workers)
                w.request_stop();
            cv.notify_all();
            workers.clear();
            if (shared_data.empty()) {
                spdlog::error("data doesn't parsed from input files");
                return std::unexpected<bool>(false);
            }
            std::sort(shared_data.begin(), shared_data.end());
            spdlog::info(std::format("Parsing finished: parsed {} lines", std::to_string(shared_data.size())));
            return shared_data;
        }
        
    private:
        void    taskGenerate(std::vector<std::string>& files) {
            spdlog::info("Input files: ");
            std::for_each(files.begin(), files.end(), [&](const auto& file)  {
                spdlog::info(std::format("  {} ({} bytes)", file, std::filesystem::file_size(file)));
                getTasks(file);
            });
        }

        void    getTasks(const std::string& file) {
            auto result = chunk_getter(file);
            if (result)
                for (auto& chunk: result.value()) {
                    std::lock_guard<std::mutex>  lock(tasks_mutex);
                    tasks.emplace(std::move(chunk));
                    cv.notify_one();
                }
        }

        void    worker_loop(std::stop_token st, ParserFunc func) {
            while (!st.stop_requested()) {
                std::unique_lock<std::mutex> lock(tasks_mutex);
                cv.wait(lock, [this, &st] { return !tasks.empty() || st.stop_requested();});
                if (st.stop_requested() && tasks.empty())
                return ;
                auto chunk = tasks.front();
                tasks.pop();
                lock.unlock();
                auto parsed = func(chunk);
                if (!parsed.empty()) {
                    std::lock_guard<std::mutex> shared_lock(shared_data_mutex);
                    shared_data.insert(shared_data.end()
                                , std::make_move_iterator(parsed.begin())
                                , std::make_move_iterator(parsed.end()));
                }
            }
        }

    private:
        std::queue<std::string> tasks;
        std::vector<Data> shared_data;
        std::vector<std::jthread> workers;
        std::mutex shared_data_mutex;
        std::mutex tasks_mutex;
        std::condition_variable cv;
        ChunkGetterFunc *chunk_getter;
};