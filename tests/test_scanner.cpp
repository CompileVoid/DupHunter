#include "duphunter/scanner.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

namespace fs = std::filesystem;

static std::size_t create_test_file(const fs::path& path, std::string_view content) {
  std::ofstream ofs(path, std::ios::binary);
  ofs.write(content.data(), static_cast<std::streamsize>(content.size()));
  return content.size();
}

TEST(ScannerTest, ScansFilesInDirectory) {
  fs::path test_dir = fs::temp_directory_path() / "duphunter_test_scan_m1";

  fs::remove_all(test_dir);
  fs::create_directories(test_dir);

  std::size_t size1 = create_test_file(test_dir / "file1.txt", "12345");
  std::size_t size2 = create_test_file(test_dir / "file2.txt", "hello world");

  fs::create_directories(test_dir / "subdir");
  std::size_t size3 =
      create_test_file(test_dir / "subdir" / "file3.txt", "recursive");

  std::size_t expected_total_size = size1 + size2 + size3;

  duphunter::ScanOptions options;
  options.root_path = test_dir;
  duphunter::ScanResult result = duphunter::scan_directory(options);

  EXPECT_EQ(result.files.size(), 3);

  EXPECT_EQ(result.errors.size(), 0);

  EXPECT_EQ(result.total_scanned_size_bytes, expected_total_size);

  bool found_file1 = false;
  for (const auto &file : result.files) {
    if (file.path.filename() == "file1.txt" && file.size_bytes == size1) {
      found_file1 = true;
      break;
    }
  }
  EXPECT_TRUE(found_file1)
      << "File 'file1.txt' with correct size was not found in results";

  fs::remove_all(test_dir);
}

TEST(ScannerTest, HandlesNonExistentPath) {
  fs::path fake_path =
      fs::temp_directory_path() / "this_path_does_not_exist_12345_xyz";

  EXPECT_FALSE(fs::exists(fake_path));

  duphunter::ScanOptions options;
  options.root_path = fake_path;

  duphunter::ScanResult result = duphunter::scan_directory(options);

  EXPECT_EQ(result.files.size(), 0);
  EXPECT_EQ(result.total_scanned_size_bytes, 0);
}