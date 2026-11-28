#pragma once

#include "scanner.h"
#include <functional>
#include <string>
#include <vector>

namespace duphunter {

struct DuplicateGroup {
  uintmax_t file_size;
  std::vector<FileEntry> files;

  uintmax_t potential_savings() const { return file_size * (files.size() - 1); }
};

struct DeduplicationResult {
  std::vector<DuplicateGroup> groups;
  size_t total_files_scanned = 0;
  size_t total_duplicates_found = 0;
  uintmax_t total_potential_savings = 0;
};

struct DeduplicationOptions {
  size_t partial_hash_size = 4096;
};

DeduplicationResult find_duplicates(const ScanResult &scan_result,
                                    const DeduplicationOptions &options = {});

} // namespace duphunter