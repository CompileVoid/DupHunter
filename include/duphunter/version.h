#pragma once
#include <string_view>

namespace duphunter {
inline constexpr std::string_view kVersion = "0.1.0";
std::string_view version() noexcept;
}