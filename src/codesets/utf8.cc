// 190 - UTF-8 Level 1
// 191 - UTF-8 Level 2
// 192 - UTF-8 Level 3
// 196 - UTF-8 without implementation level

#include "../codesets.h"

using namespace iso2022::builtin;

void
utf_8::decode (const char *&ptr,
               const char *end,
               std::u16string &out)
{
  enum { NORMAL, ESCAPE, ESC_25 } state = NORMAL;

  while (ptr < end) {
    unsigned char ch = *ptr;

    switch (state) {
    case NORMAL:
      if (_type == wSR && ch == ESC) {
        state = ESCAPE;
        ++ptr;
        break;
      }

      ++ptr;
      {
        int len;
        char32_t uc;

        if (ch < 0x80) {
          out += (char16_t)ch;
          break;
        } else if (ch < 0xc0) {
          out += u'\ufffd';
          break;
        } else if (ch < 0xe0) {
          uc = ch & 0x1f;
          len = 1;
        } else if (ch < 0xf0) {
          uc = ch & 0x0f;
          len = 2;
        } else if (ch < 0xf8) {
          uc = ch & 0x07;
          len = 3;
        } else {
          out += u'\ufffd';
          break;
        }

        while (ptr < end && len) {
          ch = *ptr;
          if ((ch & 0xc0) != 0x80)
            break;
          uc = (uc << 6) | (ch & 0x3f);
          ++ptr;
          --len;
        }

        if (len) {
          out += u'\ufffd';
          break;
        }

        if (uc >= 0xd800 && uc <= 0xdfff)
          out += u'\ufffd';
        else if (uc < 0xffff)
          out += (char16_t)uc;
        else if (uc < 0x10ffff) {
          uc -= 0x10000;
          out += (char16_t)(0xd800 | (uc >> 10));
          out += (char16_t)(0xdc00 | (uc & 0x3ff));
        } else {
          out += u'\ufffd';
        }
      }
      break;
    case ESCAPE:
      if (ch == 0x25) {
        state = ESC_25;
        ++ptr;
        break;
      }

      state = NORMAL;
      out += u'\u241b';
      // Don't eat this character
      break;
    case ESC_25:
      if (ch == 0x40)
        return;

      state = NORMAL;
      out += u"\u241b\x25";
      // Don't eat this character
      break;
    }
  }
}
