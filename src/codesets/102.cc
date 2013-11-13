// 102 - Teletex Primary Set of Graphic Characters CCITT Rec. T.61

#include "../codesets.h"

using namespace iso2022::builtin;

void
teletex::decode (unsigned char c, std::u16string &out)
{
  switch (c) {
  case 0x24:
    out += u'\xa4';
    break;
  case 0x5c:
  case 0x5e:
  case 0x60:
  case 0x7b:
  case 0x7d:
  case 0x7e:
    out += u'\ufffd';
    break;
  default:
    out += (char16_t)c; 
    break;
  }
}

void
teletex::invoke (code_element elt, std::string &out)
{
  switch (elt) {
  case ELEMENT_G0:
    out += "\x1b\x28\x75";
    break;
  case ELEMENT_G1:
    out += "\x1b\x29\x75";
    break;
  case ELEMENT_G2:
    out += "\x1b\x2a\x75";
    break;
  case ELEMENT_G3:
    out += "\x1b\x2b\x75";
    break;
  }
}

bool
teletex::encode (const char16_t *&pcs,
                 const char16_t *pcsend,
                 unsigned char base,
                 std::string &out)
{
  if (pcsend > pcs + 1)
    return false;

  char16_t uch = *pcs;
  unsigned char chout;

  switch (uch) {
  case 0xa4:
    chout = 0x24;
    break;
  case 0x24:
  case 0x5c:
  case 0x5e:
  case 0x60:
  case 0x7b:
  case 0x7d:
  case 0x7e:
    return false;
  default:
    if (uch < 0x20 || uch > 0x7f)
      return false;

    chout = uch;
  }

  out += (char)(chout + base);
  return true;
}
