#pragma once

namespace webserver::utils {

static void expect(char realChar, char expected);
static void expectDigit(char chr);
static bool isSpaceOrTab(char chr);
static bool isAsciiUppercase(char chr);

}  // namespace webserver::utils
