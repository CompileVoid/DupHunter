#include <filesystem>
#include <iostream>
#include <string>

#include "duphunter/deduplication.h"
#include "duphunter/scanner.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: duphunter <path_to_scan>\n";
    return 1;
  }

  std::filesystem::path path = argv[1];
  std::error_code ec;

  if (!std::filesystem::exists(path, ec)) {
    std::cerr << "Error: path does not exist\n";
    if (ec) {
      std::cerr << "Details: " << ec.message() << "\n";
    }
    return 1;
  }

  duphunter::ScanOptions options;
  options.root_path = path;
  duphunter::ScanResult scan_result = duphunter::scan_directory(options);

  std::cout << "Scanned " << scan_result.files.size() << " files, "
            << scan_result.errors.size() << " errors\n";

  duphunter::DeduplicationOptions dedup_options;
  duphunter::DeduplicationResult deduplicationResult =
      duphunter::find_duplicates(scan_result, dedup_options);

  std::cout << "Found " << deduplicationResult.groups.size()
            << " duplicate groups\n";

  for (const auto &group : deduplicationResult.groups) {
    std::cout << "\n=== Duplicate group (" << group.files.size()
              << " files) ===\n";
    for (const auto &file : group.files) {
      std::cout << "  " << file.path.string() << "\n";
    }
  }

  std::cout << "\nTotal duplicates: "
            << deduplicationResult.total_duplicates_found << "\n";
  std::cout << "Potential savings: "
            << deduplicationResult.total_potential_savings << " bytes\n";

  return 0;
}