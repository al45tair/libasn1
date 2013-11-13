// 162 - ISO/IEC 10646:1993, UCS-2, Level 1
// 174 - ISO/IEC 10646:1993, UCS-2, Level 2
// 176 - ISO/IEC 10646:1993, UCS-2, Level 3

#include "../codesets.h"

using namespace iso2022::builtin;

void
ucs_2::decode (const char *&ptr,
               const char *end,
               std::u16string &out)
{
  while (ptr + 2 < end) {
    char16_t ch = *ptr++ << 8;

    ch |= *ptr++;

    // Surrogates are not allowed in UCS-2
    if (ch >= 0xd800 && ch <= 0xdfff)
      ch = u'\ufffd';

    out += ch;
  }

  if (ptr < end) {
    out += u'\ufffd';
    ptr = end;
  }
}
