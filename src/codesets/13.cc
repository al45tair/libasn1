// 013 - Katakana Character Set JIS C6220-1969

#include "../codesets.h"

using namespace iso2022::builtin;

void
katakana::decode (unsigned char c, std::u16string &out)
{
  if (c == 0x20 || c >= 0x60)
    out += u'\ufffd';
  else
    out += u'\uff40' + c;
}

void
katakana::invoke (code_element elt, std::string &out)
{
  switch (elt) {
  case ELEMENT_G0: out += "\x1b\x28\x49"; break;
  case ELEMENT_G1: out += "\x1b\x29\x49"; break;
  case ELEMENT_G2: out += "\x1b\x2a\x49"; break;
  case ELEMENT_G3: out += "\x1b\x2b\x49"; break;
  }
}

bool
katakana::encode (const char16_t *&pcs,
                  const char16_t *pcsend,
                  unsigned char base,
                  std::string  &out)
{
  if (pcsend != pcs + 1)
    return false;

  if (*pcs < 0xff61 || *pcs > 0xff9f)
    return false;

  unsigned char ch = (*pcs - 0xff40) + base;
  out += (char)ch;
  return true;
}
