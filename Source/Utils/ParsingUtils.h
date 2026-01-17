#pragma once

namespace webserver::utils {

void expect(char realChar, char expected);
void expectDigit(char chr);
bool isSpaceOrTab(char chr);
bool isAsciiUppercase(char chr);

}  // namespace webserver::utils
