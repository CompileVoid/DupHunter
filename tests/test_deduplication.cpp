#include "duphunter/deduplication.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

namespace fs = std::filesystem;

static void create_file(const fs::path &path, std::string_view content) {
  std::ofstream ofs(path, std::ios::binary);
  ofs.write(content.data(), static_cast<std::streamsize>(content.size()));
}

TEST(DeduplicationTest, FindsExactDuplicates) {
  fs::path test_dir = fs::temp_directory_path() / "duphunter_test_dedup_exact";
  fs::remove_all(test_dir);
  fs::create_directories(test_dir);

  std::string content = "This is a duplicate file content for testing";
  create_file(test_dir / "file1.txt", content);
  create_file(test_dir / "file2.txt", content);
  create_file(test_dir / "file3.txt", "Different content");

  duphunter::ScanOptions scan_options;
  scan_options.root_path = test_dir;
  duphunter::ScanResult scan_result = duphunter::scan_directory(scan_options);

  duphunter::DeduplicationOptions dedup_options;
  duphunter::DeduplicationResult result =
      duphunter::find_duplicates(scan_result, dedup_options);

  EXPECT_EQ(result.groups.size(), 1);
  EXPECT_EQ(result.groups[0].files.size(), 2);
  EXPECT_EQ(result.total_duplicates_found, 2);

  EXPECT_GT(result.total_potential_savings, 0);

  // TEARDOWN
  fs::remove_all(test_dir);
}

TEST(DeduplicationTest, NoDuplicatesWhenAllUnique) {
  fs::path test_dir = fs::temp_directory_path() / "duphunter_test_dedup_unique";
  fs::remove_all(test_dir);
  fs::create_directories(test_dir);

  // Создаём файлы с уникальным содержимым
  create_file(test_dir / "file1.txt", "Content A");
  create_file(test_dir / "file2.txt", "Content B");
  create_file(test_dir / "file3.txt", "Content C");

  duphunter::ScanOptions scan_options;
  scan_options.root_path = test_dir;
  duphunter::ScanResult scan_result = duphunter::scan_directory(scan_options);

  duphunter::DeduplicationResult result =
      duphunter::find_duplicates(scan_result);

  EXPECT_EQ(result.groups.size(), 0);
  EXPECT_EQ(result.total_duplicates_found, 0);

  fs::remove_all(test_dir);
}

TEST(DeduplicationTest, SameSizeDifferentContent) {
  fs::path test_dir =
      fs::temp_directory_path() / "duphunter_test_dedup_same_size";
  fs::remove_all(test_dir);
  fs::create_directories(test_dir);

  create_file(test_dir / "file1.txt", "AAAA");
  create_file(test_dir / "file2.txt", "BBBB");

  duphunter::ScanOptions scan_options;
  scan_options.root_path = test_dir;
  duphunter::ScanResult scan_result = duphunter::scan_directory(scan_options);

  duphunter::DeduplicationResult result =
      duphunter::find_duplicates(scan_result);

  EXPECT_EQ(result.groups.size(), 0);

  fs::remove_all(test_dir);
}

TEST(DeduplicationTest, PartialHashCollisionResolvedByFullHash) {
  fs::path test_dir =
      fs::temp_directory_path() / "duphunter_test_dedup_collision";
  fs::remove_all(test_dir);
  fs::create_directories(test_dir);

  std::string prefix(5000, 'X');
  create_file(test_dir / "file1.txt", prefix + "END1");
  create_file(test_dir / "file2.txt", prefix + "END2");

  duphunter::ScanOptions scan_options;
  scan_options.root_path = test_dir;
  duphunter::ScanResult scan_result = duphunter::scan_directory(scan_options);

  duphunter::DeduplicationOptions dedup_options;
  dedup_options.partial_hash_size = 4096;

  duphunter::DeduplicationResult result =
      duphunter::find_duplicates(scan_result, dedup_options);

  EXPECT_EQ(result.groups.size(), 0);

  fs::remove_all(test_dir);
}

TEST(DeduplicationTest, ParallelHashingMatchesSequential) {
  fs::path test_dir = fs::temp_directory_path() / "duphunter_test_parallel";
  fs::remove_all(test_dir);
  fs::create_directories(test_dir);

  for (int i = 0; i < 50; ++i) {
    std::string content = "content_for_group_" + std::to_string(i) +
                          "_with_some_extra_data_to_make_it_longer";
    create_file(test_dir / ("a_" + std::to_string(i) + ".txt"), content);
    create_file(test_dir / ("b_" + std::to_string(i) + ".txt"), content);
  }

  duphunter::ScanOptions scan_opts;
  scan_opts.root_path = test_dir;
  auto scan_result = duphunter::scan_directory(scan_opts);

  duphunter::DeduplicationOptions seq_opts;
  seq_opts.max_threads = 1;
  auto seq_result = duphunter::find_duplicates(scan_result, seq_opts);

  duphunter::DeduplicationOptions par_opts;
  par_opts.max_threads = 4;
  auto par_result = duphunter::find_duplicates(scan_result, par_opts);

  EXPECT_EQ(seq_result.groups.size(), par_result.groups.size());
  EXPECT_EQ(seq_result.total_duplicates_found, par_result.total_duplicates_found);
  EXPECT_EQ(seq_result.total_potential_savings, par_result.total_potential_savings);

  fs::remove_all(test_dir);
}