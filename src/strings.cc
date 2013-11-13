#include <asn1/strings.h>

const unsigned asn1::t61_graphic_codesets[] = {
  6, 
  //87,
  102,
  103,
  // 126, 144, 150, 153, 156, 164, 165, 168
};
const unsigned asn1::t61_graphic_codeset_count 
  = sizeof(asn1::t61_graphic_codesets) / sizeof (unsigned);

const unsigned asn1::t61_default_graphic_set[] = { 102, 0, 0, 0 };
const unsigned asn1::t61_default_control_set[] = { 1, 0 }; //106, 107 };

const unsigned asn1::videotex_graphic_codesets[] = {
  13, 72, 
  // 87,
  89,
  102,
  // 126, 128, 129, 144, 150, 153, 164, 165, 168
};
const unsigned asn1::videotex_graphic_codeset_count
  = sizeof(asn1::videotex_graphic_codesets) / sizeof (unsigned);

const unsigned asn1::videotex_default_graphic_set[] = { 102, 0, 0, 0 };
const unsigned asn1::videotex_default_control_set[] = { 1, 73 };

std::string
asn1::utf16_to_utf8 (const char16_t *ptr, size_t len)
{
  std::string result;
  const char16_t *end = ptr + len;

  while (ptr < end) {
    char32_t ch = *ptr++;

    if (ch >= 0xd800 && ch <= 0xdbff) {
      if (ptr >= end)
        throw std::runtime_error("bad UTF-16 - incomplete surrogate");

      char16_t ch2 = *ptr++;

      if (ch2 < 0xdc00 || ch2 > 0xdfff)
        throw std::runtime_error("bad UTF-16 - damaged surrogate");

      ch = 0x10000 + (((ch & 0x3ff) << 10) | (ch2 & 0x3ff));
      if (ch > 0x10ffff)
        throw std::runtime_error("bad UTF-16 - codepoint > U+10FFFF");
    } else if (ch >= 0xdc00 && ch <= 0xdfff)
      throw std::runtime_error("baf UTF-16 - missing first surrogate");

    if (ch < 0x80)
      result.push_back(ch);
    else if (ch < 0x800) {
      result.push_back(0xc0|(ch >> 6));
      result.push_back(0x80|(ch & 0x3f));
    } else if (ch < 0x10000) {
      result.push_back(0xe0|(ch >> 12));
      result.push_back(0x80|((ch >> 6) & 0x3f));
      result.push_back(0x80|(ch & 0x3f));
    } else {
      result.push_back(0xf0|(ch >> 18));
      result.push_back(0x80|((ch >> 12) & 0x3f));
      result.push_back(0x80|((ch >> 6) & 0x3f));
      result.push_back(0x80|(ch & 0x3f));
    }
  }

  return result;
}

std::u16string
asn1::utf8_to_utf16 (const char *str, size_t len)
{
  std::u16string result;
  const unsigned char *ptr = (const unsigned char *)str;
  const unsigned char *end = ptr + len;

  while (ptr < end) {
    char32_t ch = *ptr++;
    unsigned count = 0;

    if (ch >= 0xf8) {
      ch = 0xfffd;
    } else if (ch >= 0xf0) {
      ch &= 0x07;
      if (!ch)
        ch = 0xfffd;
      else
        count = 3;
    } else if (ch >= 0xe0) {
      ch &= 0x0f;
      if (!ch)
        ch = 0xfffd;
      else
        count = 2;
    } else if (ch >= 0xc0) {
      ch &= 0x1f;
      if (!ch)
        ch = 0xfffd;
      else
        count = 1;
    } else if (ch >= 0x80)
      ch = 0xfffd;

    while (ptr < end && count) {
      unsigned char c = *ptr;
      if ((c & 0xc0) != 0x80)
        break;
      ch = (ch << 6) | (c & 0x3f);
      --count;
      ++ptr;
    }
     
    if (count)
      ch = 0xfffd;

    result += ch;
  }

  return result;
}
