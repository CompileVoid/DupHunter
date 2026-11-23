#include "duphunter/scanner.h"

#include <filesystem>

namespace duphunter {
ScanResult scan_directory(const ScanOptions &options) {
  ScanResult result;

  std::error_code ec_exists;
  if (!std::filesystem::exists(options.root_path, ec_exists) ||
      !std::filesystem::is_directory(options.root_path, ec_exists)) {
    if (ec_exists) {
      result.errors.push_back(
          {options.root_path, ec_exists, ec_exists.message()});
    }
    return result;
  }

  auto it = std::filesystem::recursive_directory_iterator(
      options.root_path,
      std::filesystem::directory_options::skip_permission_denied);

  for (const auto &dir_entry : it) {
    if (dir_entry.is_symlink()) {
      continue;
    }

    if (!dir_entry.is_regular_file()) {
      continue;
    }
    std::error_code ec_size;
    uintmax_t size = dir_entry.file_size(ec_size);

    if (ec_size) {
      ScanError error{dir_entry.path(), ec_size, ec_size.message()};
      result.errors.push_back(error);
      continue;
    }
    FileEntry entry{dir_entry.path(), size};
    result.files.push_back(entry);
    result.total_scanned_size_bytes += size;
  }

  return result;
}

} // namespace duphunter
