#include "duphunter/scanner.h"
#include "duphunter/thread_pool.h"
#include <filesystem>
#include <variant>
#include <vector>
#include <future>

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

  ThreadPool pool(options.max_threads);
  std::vector<std::future<FileTaskResult>> futures;
  futures.reserve(1000);

  auto it = std::filesystem::recursive_directory_iterator(
      options.root_path,
      std::filesystem::directory_options::skip_permission_denied);

  for (const auto &dir_entry : it) {
    if (options.stop_token.stop_requested()) {
      result.cancelled = true;
      break;
    }

    if (dir_entry.is_symlink() || !dir_entry.is_regular_file()) {
      continue;
    }

    futures.push_back(
        pool.submit([path = dir_entry.path()]() -> FileTaskResult {
          std::error_code ec;
          uintmax_t size = std::filesystem::file_size(path, ec);
          if (ec) {
            return ScanError{path, ec, ec.message()};
          }
          return FileEntry{path, size};
        }));
  }

  for (auto &fut : futures) {
    FileTaskResult task_result = fut.get();

    std::visit(
        [&result](auto &&arg) {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, FileEntry>) {
            result.files.push_back(std::move(arg));
            result.total_scanned_size_bytes += arg.size_bytes;
          } else if constexpr (std::is_same_v<T, ScanError>) {
            result.errors.push_back(std::move(arg));
          }
        },
        task_result);
  }

  return result;
}

} // namespace duphunter
