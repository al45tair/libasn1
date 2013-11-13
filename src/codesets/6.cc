// 006 - ISO 646, USA Version X3.4 - 1968

#include "../codesets.h"

using namespace iso2022::builtin;

void
ascii::decode (unsigned char c, std::u16string &out)
{
  out += (char16_t)c;
}

void
ascii::invoke (code_element elt, std::string &out)
{
  switch (elt) {
  case ELEMENT_G0:
    out += "\x1b\x28\x42";
    break;
  case ELEMENT_G1:
    out += "\x1b\x29\x42";
    break;
  case ELEMENT_G2:
    out += "\x1b\x2a\x42";
    break;
  case ELEMENT_G3:
    out += "\x1b\x2b\x42";
    break;
  }
}

bool
ascii::encode (const char16_t *&pcs,
               const char16_t *pcsend,
               unsigned char base,
               std::string &out)
{
  if (pcsend > pcs + 1)
    return false;
  if (*pcs < 0x20 || *pcs > 0x7f)
    return false;
  out += (char)(*pcs + base);
  return true;
}
