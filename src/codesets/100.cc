// 100 - Right-hand Part of Latin Alphabet No.1 ISO 8859/1, ECMA-94

#include "../codesets.h"

using namespace iso2022::builtin;

void
iso_8859_1::decode (unsigned char c, std::u16string &out)
{
  out += (char16_t)(c + 0x80);
}

void
iso_8859_1::invoke (code_element elt, std::string &out)
{
  switch (elt) {
  case ELEMENT_G0:
    throw std::runtime_error("Attempted to invoke ISO-8859-1 in G0");
  case ELEMENT_G1:
    out += "\x1b\x2d\x41";
    break;
  case ELEMENT_G2:
    out += "\x1b\x2e\x41";
    break;
  case ELEMENT_G3:
    out += "\x1b\x2f\x41";
    break;
  }
}

bool
iso_8859_1::encode (const char16_t *&pcs,
                    const char16_t *pcsend,
                    unsigned char base,
                    std::string &out)
{
  if (pcsend == pcs + 1) {
    if (*pcs >= 0xa0 && *pcs <= 0xff) {
      out += *pcs - 0x80 + base;
      return true;
    }

    return false;
  }

  if (pcsend > pcs + 2)
    return false;
  
  char16_t ch = *pcs++;
  char cse = ch & 0x20;
  char chout;

  switch (ch) {
  case 'A': case 'a':
    switch (*pcs) {
    case 0x0300: chout = (char)0xc0 | cse; break;
    case 0x0301: chout = (char)0xc1 | cse; break;
    case 0x0302: chout = (char)0xc2 | cse; break;
    case 0x0303: chout = (char)0xc3 | cse; break;
    case 0x0308: chout = (char)0xc4 | cse; break;
    case 0x030a: chout = (char)0xc5 | cse; break;
    default: return false;
    }
    break;
  case 'C': case 'c':
    if (*pcs == 0x0327) { chout = (char)0xc7 | cse; break; }
    return false;
  case 'E': case 'e':
    switch (*pcs) {
    case 0x0300: chout = (char)0xc8 | cse; break;
    case 0x0301: chout = (char)0xc9 | cse; break;
    case 0x0302: chout = (char)0xca | cse; break;
    case 0x0308: chout = (char)0xcb | cse; break;
    default: return false;
    }
    break;
  case 'I': case 'i': case 0x0131:
    switch (*pcs) {
    case 0x0300: chout = (char)0xcc | cse; break;
    case 0x0301: chout = (char)0xcd | cse; break;
    case 0x0302: chout = (char)0xce | cse; break;
    case 0x0308: chout = (char)0xcf | cse; break;
    default: return false;
    }
    break;
  case 'N': case 'n':
    if (*pcs == 0x0303) { chout = (char)0xd1 | cse; break; }
    return false;
  case 'O': case 'o':
    switch (*pcs) {
    case 0x0300: chout = (char)0xd2 | cse; break;
    case 0x0301: chout = (char)0xd3 | cse; break;
    case 0x0302: chout = (char)0xd4 | cse; break;
    case 0x0303: chout = (char)0xd5 | cse; break;
    case 0x0308: chout = (char)0xd6 | cse; break;
    default: return false;
    }
    break;
  case 'U': case 'u':
    switch (*pcs) {
    case 0x0300: chout = (char)0xd9 | cse; break;
    case 0x0301: chout = (char)0xda | cse; break;
    case 0x0302: chout = (char)0xdb | cse; break;
    case 0x0308: chout = (char)0xdc | cse; break;
    default: return false;
    }
    break;
  case 'Y': case 'y':
    if (*pcs == 0x0301) { chout = (char)0xdd | cse; break; }
    if (ch == 'y' && *pcs == 0x0308) { chout = (char)0xff; break; }
    return false;

  default:
    return false;
  }

  out += chout - 0x80 + base;

  return true;
}
