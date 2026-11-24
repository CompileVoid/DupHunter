#pragma once

#include <filesystem>
#include <stop_token>
#include <string>
#include <system_error>
#include <variant>
#include <vector>

namespace duphunter {

struct FileEntry {
  std::filesystem::path path;
  uintmax_t size_bytes;
};

struct ScanError {
  std::filesystem::path path;
  std::error_code ec;
  std::string message;
};

struct ScanResult {
  std::vector<FileEntry> files;
  std::vector<ScanError> errors;
  uintmax_t total_scanned_size_bytes = 0;
  bool cancelled = false;
};

struct ScanOptions {
  std::filesystem::path root_path;
  size_t max_threads = 4;
  std::stop_token stop_token;
};

using FileTaskResult = std::variant<FileEntry, ScanError>;

ScanResult scan_directory(const ScanOptions &options);

} // namespace duphunter