#include <gtest/gtest.h>
#include "duphunter/version.h"

TEST(Smoke, VersionIsSet) {
  EXPECT_EQ(duphunter::version(), "0.1.0");
}

TEST(Smoke, VersionNotEmpty) {
  EXPECT_FALSE(duphunter::version().empty());
}