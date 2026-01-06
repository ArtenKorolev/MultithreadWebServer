#include "ParsingUtils.h"

#include <fmt/core.h>

#include <stdexcept>

#define INLINE __attribute__((always_inline)) inline

namespace webserver::utils {

INLINE void expect(const char realChar, const char expected) {
  if (realChar != expected) {
    throw std::runtime_error(
        fmt::format("expected '{}' but got '{}'", expected, realChar));
  }
}

INLINE void expectDigit(char chr) {
  if (chr < '0' || chr > '9') {
    throw std::runtime_error(fmt::format("expected digit but got '{}'", chr));
  }
}

INLINE bool isSpaceOrTab(const char chr) {
  return chr == ' ' || chr == '\t';
}

INLINE bool isAsciiUppercase(const char chr) {
  return chr >= 'A' && chr <= 'Z';
}

}  // namespace webserver::utils
