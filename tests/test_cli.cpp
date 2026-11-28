#include <gtest/gtest.h>

#include "duphunter/cli.h"

TEST(CliTest, FormatSizeBytes) {
    EXPECT_EQ(duphunter::format_size(0), "0 B");
    EXPECT_EQ(duphunter::format_size(512), "512 B");
    EXPECT_EQ(duphunter::format_size(1023), "1023 B");
}

TEST(CliTest, FormatSizeKilobytes) {
    EXPECT_EQ(duphunter::format_size(1024), "1.00 KB");
    EXPECT_EQ(duphunter::format_size(1536), "1.50 KB");
    EXPECT_EQ(duphunter::format_size(10240), "10.00 KB");
}

TEST(CliTest, FormatSizeMegabytes) {
    EXPECT_EQ(duphunter::format_size(1024ULL * 1024), "1.00 MB");
    EXPECT_EQ(duphunter::format_size(1024ULL * 1024 * 25 / 10), "2.50 MB");
}

TEST(CliTest, FormatSizeGigabytes) {
    EXPECT_EQ(duphunter::format_size(1024ULL * 1024 * 1024), "1.00 GB");
}

TEST(CliTest, ParseSizeStringPlainNumber) {
    EXPECT_EQ(duphunter::parse_size_string("1024"), 1024);
    EXPECT_EQ(duphunter::parse_size_string("0"), 0);
}

TEST(CliTest, ParseSizeStringWithSuffix) {
    EXPECT_EQ(duphunter::parse_size_string("1KB"), 1024);
    EXPECT_EQ(duphunter::parse_size_string("1MB"), 1024ULL * 1024);
    EXPECT_EQ(duphunter::parse_size_string("2GB"), 2ULL * 1024 * 1024 * 1024);
    EXPECT_EQ(duphunter::parse_size_string("512B"), 512);
}

TEST(CliTest, ParseSizeStringInvalid) {
    EXPECT_EQ(duphunter::parse_size_string(""), 0);
    EXPECT_EQ(duphunter::parse_size_string("abc"), 0);
    EXPECT_EQ(duphunter::parse_size_string("12XB"), 0);
}

TEST(CliTest, ParseMinSize) {
    char arg0[] = "prog";
    char arg1[] = "/tmp";
    char arg2[] = "--min-size";
    char arg3[] = "1024";
    char* argv[] = {arg0, arg1, arg2, arg3};

    duphunter::CliOptions opts;
    EXPECT_TRUE(duphunter::parse_cli(4, argv, opts));
    EXPECT_EQ(opts.root_path, "/tmp");
    EXPECT_EQ(opts.min_size_bytes, 1024);
}

TEST(CliTest, ParseMinSizeWithSuffix) {
    char arg0[] = "prog";
    char arg1[] = "/tmp";
    char arg2[] = "--min-size";
    char arg3[] = "1MB";
    char* argv[] = {arg0, arg1, arg2, arg3};

    duphunter::CliOptions opts;
    EXPECT_TRUE(duphunter::parse_cli(4, argv, opts));
    EXPECT_EQ(opts.min_size_bytes, 1024ULL * 1024);
}

TEST(CliTest, ParseExclude) {
    char arg0[] = "prog";
    char arg1[] = "/tmp";
    char arg2[] = "--exclude";
    char arg3[] = "build,vcpkg,.git";
    char* argv[] = {arg0, arg1, arg2, arg3};

    duphunter::CliOptions opts;
    EXPECT_TRUE(duphunter::parse_cli(4, argv, opts));
    EXPECT_EQ(opts.exclude_patterns.size(), 3);
    EXPECT_EQ(opts.exclude_patterns[0], "build");
    EXPECT_EQ(opts.exclude_patterns[1], "vcpkg");
    EXPECT_EQ(opts.exclude_patterns[2], ".git");
}

TEST(CliTest, ParseNoColor) {
    char arg0[] = "prog";
    char arg1[] = "/tmp";
    char arg2[] = "--no-color";
    char* argv[] = {arg0, arg1, arg2};

    duphunter::CliOptions opts;
    EXPECT_TRUE(duphunter::parse_cli(3, argv, opts));
    EXPECT_TRUE(opts.no_color);
}

TEST(CliTest, ParseHelp) {
    char arg0[] = "prog";
    char arg1[] = "--help";
    char* argv[] = {arg0, arg1};

    duphunter::CliOptions opts;
    EXPECT_FALSE(duphunter::parse_cli(2, argv, opts));
}

TEST(CliTest, NoArgsReturnsFalse) {
    char arg0[] = "prog";
    char* argv[] = {arg0};

    duphunter::CliOptions opts;
    EXPECT_FALSE(duphunter::parse_cli(1, argv, opts));
}

TEST(CliTest, ColorDisabled) {
    duphunter::color::set_enabled(false);
    EXPECT_EQ(duphunter::color::red(), "");
    EXPECT_EQ(duphunter::color::reset(), "");

    duphunter::color::set_enabled(true);
    EXPECT_NE(duphunter::color::red(), "");
}