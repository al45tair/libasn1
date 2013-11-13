// 163 - ISO/IEC 10646:1993, UCS-4, Level 1
// 175 - ISO/IEC 10646:1993, UCS-4, Level 2
// 177 - ISO/IEC 10646:1993, UCS-4, Level 3

#include "../codesets.h"

using namespace iso2022::builtin;

void
ucs_4::decode (const char *&ptr,
               const char *end,
               std::u16string &out)
{
  while (ptr + 3 < end) {
    char32_t ch = *ptr++ << 24;

    ch |= *ptr++ << 16;
    ch |= *ptr++ << 8;
    ch |= *ptr++;
    
    // Surrogates are not allowed in UCS-4
    if (ch >= 0xd800 && ch <= 0xdfff)
      ch = u'\ufffd';

    // Code points outside the Unicode code space are not allowed
    if (ch > 0x10ffff)
      ch = u'\ufffd';

    if (ch < 0xffff)
      out += (char16_t)ch;
    else {
      ch -= 0x10000;
      out += (char16_t)(0xd800 | (ch >> 10));
      out += (char16_t)(0xdc00 | (ch & 0x3ff));
    }
  }

  if (ptr < end) {
    out += u'\ufffd';
    ptr = end;
  }
}
