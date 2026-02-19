# CSV Median Calculator - Stats Logger

A high-performance C++ application for concurrent parsing and statistical analysis of large CSV log files. This project demonstrates efficient multi-threaded file processing using a thread pool architecture.

## Overview

The **Stats Logger** application processes large CSV-formatted trading log files and calculates statistical metrics such as median values. It utilizes a concurrent thread pool to parse multiple file chunks simultaneously, improving throughput on multi-core systems.

### Key Features

- **Concurrent File Parsing**: Uses a configurable thread pool for parallel CSV processing
- **Efficient Memory Management**: Streams and chunks large files to manage memory efficiently
- **Structured Logging**: Built-in logging using spdlog for debugging and monitoring
- **Configuration Management**: TOML-based configuration for flexible deployment
- **Command-Line Interface**: Boost program_options for flexible command-line argument handling
- **Modern C++**: Utilizes C++23 features including `std::expected` for error handling

## Project Structure

```
csv_median_calculator/
├── CMakeLists.txt              # Build configuration
├── config.toml                 # Application configuration
├── README.md                   # This file
├── include/                    # Header files
│   ├── concurrent_log_parser.hpp    # Thread pool implementation
│   ├── toml_config_parser.hpp       # Configuration file handling
│   ├── logger.hpp                   # Statistics logger
│   └── parser_csv.hpp           # CSV parsing utilities
├── src/                        # Implementation files
│   ├── main.cpp                     # Application entry point
│   ├── concurrent_log_parser.cpp    # Thread pool implementation
│   ├── toml_config_parser.cpp       # Config file parser
│   ├── logger.cpp                   # Statistics logger
│   └── parser_csv.cpp               # CSV parser implementation
└── examples/                   # Example files
    ├── input/                  # Input CSV files
    └── output/                 # Output results
```

## Architecture

### Core Components

#### 1. **Parser_csv**
Parses individual CSV log entries with the following fields:
- `receive_ts`: Reception timestamp (uint64_t)
- `exchange_ts`: Exchange timestamp (uint64_t)
- `price`: Asset price (double)
- `quantity`: Transaction quantity (double)
- `side`: Transaction side (string)
- `rebuild`: Rebuild flag (int)

#### 2. **ConcurrentLogParser**
Thread pool for concurrent processing:
- Configurable number of worker threads
- Task queue for distributing file chunks
- Thread-safe data aggregation using mutexes
- Templated design for flexible parsing functions

#### 3. **ConfigLoader**
Manages configuration from TOML files:
- Input/output directory paths
- Filename filtering patterns
- Extensible configuration structure

#### 4. **Logger**
Computes and logs statistical metrics using Boost.Accumulator:
- **Median**: Calculates the median value of prices
- **Minimum**: Tracks minimum price value
- **Maximum**: Tracks maximum price value
- Statistical analysis with boost::accumulator_set
- Result serialization to output files

## Building the Project

### Prerequisites

#### System Requirements
- **C++ Compiler**: Supporting C++20 standard (GCC 11+, Clang 13+, or MSVC 2019+)
- **CMake**: Version 3.23 or later
- **Linux/Unix**: For full compatibility (tested on Ubuntu 20.04+)

#### Required Libraries
- **Boost**: Version 1.75 or later
  - Required components:
    - `program_options`: Command-line argument parsing
    - `accumulator`: Statistical calculations (median, min, max)

#### External Dependencies (auto-fetched via CMake)
- **spdlog**: v1.11+ - Fast C++ logging library
- **tomlplusplus**: v3.3+ - TOML configuration file parser

#### Installation Commands

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    libboost-all-dev
```

**Fedora/RHEL:**
```bash
sudo dnf install -y \
    gcc-c++ \
    cmake \
    boost-devel
```

**macOS (with Homebrew):**
```bash
brew install cmake boost
```

### Build Steps

1. **Clone and navigate to the project**:
```bash
cd stats_logger
```

2. **Create build directory**:
```bash
mkdir -p build && cd build
```

3. **Configure with CMake**:
```bash
cmake ..
```

4. **Build the executable**:
```bash
cmake --build .
```

The executable `stats_logger` will be created in the build directory.

### Alternative: Direct Make Build

```bash
cd build
make
```

## Usage

### Basic Usage

Run the application with the default configuration:
```bash
./stats_logger
```

### Specify Configuration File

Use custom configuration file:
```bash
./stats_logger --config /path/to/config.toml
```

Or using short option:
```bash
./stats_logger --cfg /path/to/config.toml
```

### Configuration File Format

Edit `config.toml`:
```toml
[main]
    input = "./examples/input"      # Input directory path
    output = "./examples/output/"   # Output directory path
    filename_mask = []              # Optional: filename filters
```

## Implementation Details

### Statistical Calculation

The application uses **Boost.Accumulator** library for efficient statistical computations:

```cpp
boost::accumulators::accumulator_set<double,
    boost::accumulators::features<
        boost::accumulators::tag::median,
        boost::accumulators::tag::min,
        boost::accumulators::tag::max
    >
> acc;
```

**Calculated Statistics:**
- **Median Price**: Middle value of all price entries (50th percentile)
- **Minimum Price**: Lowest price encountered in the dataset
- **Maximum Price**: Highest price encountered in the dataset

### Threading Model

The application uses a producer-consumer pattern with a configurable thread pool:

1. **Main Thread**: Reads input files and queues chunks for processing
2. **Worker Threads**: Retrieve tasks from queue, parse CSV chunks, and aggregate results
3. **Thread Synchronization**: Uses `std::mutex`, `std::condition_variable`, and `std::jthread` with `std::stop_token` for safe multi-threaded operations
4. **Data Aggregation**: Thread-safe collection of parsed data from all worker threads

Default worker count: **4 threads** (configurable in source code)

### Error Handling

- Uses `std::expected<T, E>` for error-aware return types (C++23)
- Structured exception handling with detailed error logging
- Validates input files and configuration parameters

### Logging

Built-in logging via spdlog at multiple levels:
- **Info**: Application startup and processing milestones
- **Error**: Exception details and failure conditions

## Example Workflow

1. **Configuration**: Set input/output paths in `config.toml`
2. **Execution**: Run the application
3. **Processing**: Application concurrently parses CSV files from the input directory
4. **Results**: Statistical analysis output written to the output directory

## Performance Characteristics

- **Concurrency**: 4-threaded default processing
- **Memory Efficiency**: Chunk-based file streaming
- **Throughput**: Scales with available CPU cores
- **Scalability**: Thread pool size adjustable via code modification

## Compiler Options

The project applies strict compilation flags:
- **MSVC**: `/W4` (Warning level 4)
- **GCC/Clang**: `-Wall -Wextra -pedantic`

## Version

Current Version: **1.1.0**

## License

[Specify your license here if applicable]

## Contributing

[Contribution guidelines if applicable]

## Support

For issues or questions, review the source code comments and logging output for diagnostic information.

## Resources and References

This project utilizes the following open-source libraries and technologies:

### External Libraries
- **Boost C++ Libraries** (boost.org)
  - `Boost.Program_options`: For command-line argument parsing
  - `Boost.Accumulator`: For efficient statistical computations (median, min, max)
  
- **spdlog** (github.com/gabime/spdlog)
  - Fast C++ logging library with multiple sinks and formatters
  
- **tomlplusplus** (github.com/marzer/tomlplusplus)
  - Header-only TOML configuration file parser

### C++ Standards and Features
- C++20 standard library features
- Modern C++ best practices including RAII, move semantics, and template metaprogramming
- Concepts for compile-time constraint checking

### Concurrency Patterns
- Producer-consumer pattern with thread pool architecture
- Thread-safe data structures using standard synchronization primitives
- `std::jthread` with `std::stop_token` for clean thread management

## Author

**Assam04**

Development Date: February 2026
