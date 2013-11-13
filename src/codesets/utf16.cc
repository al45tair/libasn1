// 193 - UTF-16 Level 1
// 194 - UTF-16 Level 2
// 195 - UTF-16 Level 3

#include "../codesets.h"

using namespace iso2022::builtin;

void
utf_16::decode (const char *&ptr,
                const char *end,
                std::u16string &out)
{
  while (ptr < end) {
    char16_t ch = *ptr++ << 8;

    if (ptr >= end) {
      out += u'\ufffd';
      break;
    }

    ch |= *ptr++;

    out += ch;
  }
}
