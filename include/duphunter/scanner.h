#pragma once

#include <filesystem>
#include <string>
#include <system_error>
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
};

struct ScanOptions {
  std::filesystem::path root_path;
  bool follow_symlinks = false;
};

ScanResult scan_directory(const ScanOptions &options);

} // namespace duphunter