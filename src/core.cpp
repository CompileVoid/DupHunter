#include "duphunter/version.h"

namespace duphunter {
std::string_view version() noexcept { return kVersion; }
}