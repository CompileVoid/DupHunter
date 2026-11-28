#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace duphunter {

struct CliOptions {
  std::filesystem::path root_path;
  uintmax_t min_size_bytes = 0;
  std::vector<std::string> exclude_patterns;
  bool no_color = false;
};

bool parse_cli(int argc, char* argv[], CliOptions& options);

std::string format_size(uintmax_t bytes);

uintmax_t parse_size_string(const std::string& str);

namespace color {
void set_enabled(bool enabled);
bool is_enabled();
std::string reset();
std::string red();
std::string green();
std::string yellow();
std::string cyan();
std::string bold();
}

}  // namespace duphunter