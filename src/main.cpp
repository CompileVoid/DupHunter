#include <filesystem>
#include <iostream>
#include <string>
#include <chrono>

#include "duphunter/cli.h"
#include "duphunter/deduplication.h"
#include "duphunter/scanner.h"

#include <thread>

#ifdef _WIN32
#include <windows.h>

void enable_ansi_colors() {
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hOut == INVALID_HANDLE_VALUE) {
    return;
  }
  DWORD dwMode = 0;
  if (!GetConsoleMode(hOut, &dwMode)) {
    return;
  }
  dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  SetConsoleMode(hOut, dwMode);
}
#else
void enable_ansi_colors() {}
#endif

int main(int argc, char *argv[]) {
  enable_ansi_colors();
  auto start_time = std::chrono::steady_clock::now();

  duphunter::CliOptions cli_opts;
  if (!duphunter::parse_cli(argc, argv, cli_opts)) {
    return 1;
  }

  if (cli_opts.no_color) {
    duphunter::color::set_enabled(false);
  }

  std::error_code ec;
  if (!std::filesystem::exists(cli_opts.root_path, ec)) {
    std::cerr << duphunter::color::red()
              << "Error: " << duphunter::color::reset()
              << "path does not exist: " << cli_opts.root_path.string() << "\n";
    return 1;
  }

  std::cout << duphunter::color::cyan() << "Scanning "
            << duphunter::color::reset() << duphunter::color::bold()
            << cli_opts.root_path.string() << duphunter::color::reset()
            << "...\n";

  duphunter::ScanOptions scan_opts;
  scan_opts.root_path = cli_opts.root_path;

  duphunter::ScanResult scan_result = duphunter::scan_directory(scan_opts);

  if (cli_opts.min_size_bytes > 0 || !cli_opts.exclude_patterns.empty()) {
    std::vector<duphunter::FileEntry> filtered;
    for (const auto &file : scan_result.files) {

      if (file.size_bytes < cli_opts.min_size_bytes) {
        continue;
      }

      bool excluded = false;
      std::string path_str = file.path.string();
      for (const auto &pattern : cli_opts.exclude_patterns) {
        if (path_str.find(pattern) != std::string::npos) {
          excluded = true;
          break;
        }
      }
      if (excluded) {
        continue;
      }

      filtered.push_back(file);
    }
    scan_result.files = std::move(filtered);

    uintmax_t total = 0;
    for (const auto &file : scan_result.files) {
      total += file.size_bytes;
    }
    scan_result.total_scanned_size_bytes = total;
  }

  std::cout << "Found " << duphunter::color::green() << scan_result.files.size()
            << duphunter::color::reset() << " files ("
            << duphunter::format_size(scan_result.total_scanned_size_bytes)
            << ")\n";

  if (!scan_result.errors.empty()) {
    std::cout << duphunter::color::yellow() << scan_result.errors.size()
              << " access errors" << duphunter::color::reset() << "\n";
  }

  duphunter::DeduplicationOptions dedup_opts;
  dedup_opts.max_threads = std::thread::hardware_concurrency();
  if (dedup_opts.max_threads == 0) {
    dedup_opts.max_threads = 4;
  }
  duphunter::DeduplicationResult result =
      duphunter::find_duplicates(scan_result, dedup_opts);

  std::cout << "\n"
            << duphunter::color::bold()
            << "=== RESULTS ===" << duphunter::color::reset() << "\n";
  std::cout << "Found " << duphunter::color::red() << result.groups.size()
            << duphunter::color::reset() << " duplicate groups ("
            << result.total_duplicates_found << " files, "
            << duphunter::format_size(result.total_potential_savings)
            << " potential savings)\n\n";

  int group_num = 0;
  for (const auto &group : result.groups) {
    ++group_num;
    std::cout << duphunter::color::cyan() << "--- Group " << group_num << " ("
              << group.files.size() << " files, "
              << duphunter::format_size(group.file_size) << " each, "
              << "waste: " << duphunter::format_size(group.potential_savings())
              << ") ---" << duphunter::color::reset() << "\n";

    for (size_t i = 0; i < group.files.size(); ++i) {
      const auto &file = group.files[i];
      if (i == 0) {
        std::cout << duphunter::color::green() << "  [KEEP] "
                  << duphunter::color::reset();
      } else {
        std::cout << duphunter::color::red() << "  [DUP]  "
                  << duphunter::color::reset();
      }
      std::cout << file.path.string() << "\n";
    }
    std::cout << "\n";
  }

  std::cout << duphunter::color::bold()
            << "=== SUMMARY ===" << duphunter::color::reset() << "\n";
  std::cout << "Total files scanned: " << result.total_files_scanned << "\n";
  std::cout << "Duplicate groups:    " << result.groups.size() << "\n";
  std::cout << "Total duplicates:    " << result.total_duplicates_found << "\n";
  std::cout << duphunter::color::green() << duphunter::color::bold()
            << "Potential savings:   "
            << duphunter::format_size(result.total_potential_savings)
            << duphunter::color::reset() << "\n";

  auto end_time = std::chrono::steady_clock::now();
  auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        end_time - start_time)
                        .count();

  std::cout << "\n"
            << duphunter::color::yellow() << duphunter::color::bold()
            << "Time elapsed: " << elapsed_ms << " ms"
            << duphunter::color::reset() << "\n";

  return 0;
}