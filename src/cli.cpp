#include "duphunter/cli.h"

#include <iomanip>
#include <iostream>
#include <sstream>

namespace {

bool colors_enabled = true;

std::vector<std::string> split_string(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

void print_usage() {
    std::cerr << "Usage: duphunter <path> [options]\n"
              << "\n"
              << "Options:\n"
              << "  --min-size <size>    Skip files smaller than size (e.g., 1KB, 1MB, 1024)\n"
              << "  --exclude <patterns> Skip paths containing patterns (comma-separated)\n"
              << "  --no-color           Disable colored output\n"
              << "  -h, --help           Show this help message\n";
}

}  // namespace

namespace duphunter {

bool parse_cli(int argc, char* argv[], CliOptions& options) {
    if (argc < 2) {
        print_usage();
        return false;
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            print_usage();
            return false;
        }

        if (arg == "--min-size") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --min-size requires a value\n";
                return false;
            }
            ++i;
            options.min_size_bytes = parse_size_string(argv[i]);
            if (options.min_size_bytes == 0 && argv[i] != std::string("0")) {
                std::cerr << "Error: invalid size format: " << argv[i] << "\n";
                return false;
            }
        } else if (arg == "--exclude") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --exclude requires a value\n";
                return false;
            }
            ++i;
            options.exclude_patterns = split_string(argv[i], ',');
        } else if (arg == "--no-color") {
            options.no_color = true;
        } else if (arg[0] == '-') {
            std::cerr << "Error: unknown option: " << arg << "\n";
            print_usage();
            return false;
        } else {
            if (!options.root_path.empty()) {
                std::cerr << "Error: multiple paths specified\n";
                return false;
            }
            options.root_path = arg;
        }
    }

    if (options.root_path.empty()) {
        std::cerr << "Error: path not specified\n";
        print_usage();
        return false;
    }

    return true;
}

std::string format_size(uintmax_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    constexpr int num_units = 5;

    if (bytes == 0) {
        return "0 B";
    }

    int unit_index = 0;
    double size = static_cast<double>(bytes);

    while (size >= 1024.0 && unit_index < num_units - 1) {
        size /= 1024.0;
        ++unit_index;
    }

    std::ostringstream oss;
    if (unit_index == 0) {
        oss << bytes << " " << units[unit_index];
    } else {
        oss << std::fixed << std::setprecision(2) << size << " " << units[unit_index];
    }
    return oss.str();
}

uintmax_t parse_size_string(const std::string& str) {
    if (str.empty()) {
        return 0;
    }

    size_t pos = 0;
    uintmax_t value = 0;

    while (pos < str.size() && std::isdigit(static_cast<unsigned char>(str[pos]))) {
        value = value * 10 + static_cast<uintmax_t>(str[pos] - '0');
        ++pos;
    }

    if (pos == 0) {
        return 0;
    }

    std::string suffix = str.substr(pos);
    if (suffix.empty()) {
        return value;
    }

    if (suffix == "B" || suffix == "b") {
        return value;
    }
    if (suffix == "KB" || suffix == "kb" || suffix == "K" || suffix == "k") {
        return value * 1024;
    }
    if (suffix == "MB" || suffix == "mb" || suffix == "M" || suffix == "m") {
        return value * 1024 * 1024;
    }
    if (suffix == "GB" || suffix == "gb" || suffix == "G" || suffix == "g") {
        return value * 1024 * 1024 * 1024;
    }
    if (suffix == "TB" || suffix == "tb" || suffix == "T" || suffix == "t") {
        return value * 1024 * 1024 * 1024 * 1024;
    }

    return 0;
}

namespace color {

void set_enabled(bool enabled) {
    colors_enabled = enabled;
}

bool is_enabled() {
    return colors_enabled;
}

std::string reset() {
    return colors_enabled ? "\033[0m" : "";
}

std::string red() {
    return colors_enabled ? "\033[31m" : "";
}

std::string green() {
    return colors_enabled ? "\033[32m" : "";
}

std::string yellow() {
    return colors_enabled ? "\033[33m" : "";
}

std::string cyan() {
    return colors_enabled ? "\033[36m" : "";
}

std::string bold() {
    return colors_enabled ? "\033[1m" : "";
}

}  // namespace color

}  // namespace duphunter