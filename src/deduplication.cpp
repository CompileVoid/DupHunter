#include "duphunter/deduplication.h"

#include <filesystem>
#include <fstream>
#include <limits>
#include <unordered_map>
#include <vector>

namespace {
uint64_t
compute_file_hash(const std::filesystem::path &path,
                  size_t max_bytes = std::numeric_limits<size_t>::max()) {
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    return 0;
  }

  constexpr size_t buffer_size = 64 * 1024;
  std::vector<char> buffer(buffer_size);

  uint64_t hash = 14695981039346656037ULL;
  constexpr uint64_t prime = 1099511628211ULL;

  size_t total_read = 0;
  while (
      file.read(buffer.data(), static_cast<std::streamsize>(buffer.size())) ||
      file.gcount() > 0) {
    size_t bytes_read = static_cast<size_t>(file.gcount());

    if (max_bytes != std::numeric_limits<size_t>::max() &&
        total_read + bytes_read > max_bytes) {
      bytes_read = max_bytes - total_read;
    }

    for (size_t i = 0; i < bytes_read; ++i) {
      hash ^= static_cast<uint64_t>(static_cast<unsigned char>(buffer[i]));
      hash *= prime;
    }

    total_read += bytes_read;

    if (max_bytes != std::numeric_limits<size_t>::max() &&
        total_read >= max_bytes) {
      break;
    }
  }

  return hash;
}

uint64_t compute_partial_hash(const std::filesystem::path &path,
                              size_t max_bytes) {
  return compute_file_hash(path, max_bytes);
}

uint64_t compute_full_hash(const std::filesystem::path &path) {
  return compute_file_hash(path, std::numeric_limits<size_t>::max());
}

} // namespace

namespace duphunter {

DeduplicationResult find_duplicates(const ScanResult &scan_result,
                                    const DeduplicationOptions &options) {
  DeduplicationResult result;
  result.total_files_scanned = scan_result.files.size();

  std::unordered_map<uintmax_t, std::vector<FileEntry>> groups_by_size;
  for (const auto &file : scan_result.files) {
    groups_by_size[file.size_bytes].push_back(file);
  }

  std::vector<std::vector<FileEntry>> size_candidates;
  size_candidates.reserve(groups_by_size.size());
  for (auto &[size, files] : groups_by_size) {
    if (files.size() >= 2) {
      size_candidates.push_back(std::move(files));
    }
  }

  std::vector<std::vector<FileEntry>> partial_hash_candidates;

  for (const auto &size_group : size_candidates) {
    std::unordered_map<uint64_t, std::vector<FileEntry>> groups_by_partial_hash;

    for (const auto &file : size_group) {
      auto partialHash =
          compute_partial_hash(file.path, options.partial_hash_size);
      groups_by_partial_hash[partialHash].push_back(file);
    }

    for (auto &[pHash, files] : groups_by_partial_hash) {
      if (files.size() >= 2) {
        partial_hash_candidates.push_back(std::move(files));
      }
    }
  }

  std::vector<std::vector<FileEntry>> full_hash_candidates;

  for (const auto &hash_group : partial_hash_candidates) {
    std::unordered_map<uint64_t, std::vector<FileEntry>> groups_by_full_hash;

    for (const auto &file : hash_group) {
      auto full_hash = compute_full_hash(file.path);
      groups_by_full_hash[full_hash].push_back(file);
    }

    for (auto &[fHash, files] : groups_by_full_hash) {
      if (files.size() >= 2) {
        full_hash_candidates.push_back(std::move(files));
      }
    }
  }

  for (auto &files : full_hash_candidates) {
    DuplicateGroup group;
    group.file_size = files[0].size_bytes;
    group.files = std::move(files);

    result.total_duplicates_found += group.files.size();
    result.total_potential_savings += group.potential_savings();
    result.groups.push_back(std::move(group));
  }

  return result;
}

} // namespace duphunter