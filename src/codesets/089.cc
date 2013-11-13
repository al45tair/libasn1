// 089 - 7-bit Arabic Code for Information Interchange, Arab standard ASMO-449, ISO 9036

#include "../codesets.h"

using namespace iso2022::builtin;

void
arabic::decode (unsigned char c, std::u16string &out)
{
  char16_t uch = 0xfffd;

  if ((c >= 0x20 && c <= 0x23)
      || (c >= 0x25 && c <= 0x2b)
      || (c >= 0x2d && c <= 0x3a)
      || (c >= 0x3c && c <= 0x3e)
      || c == 0x40
      || (c >= 0x5b && c <= 0x5f)
      || (c >= 0x7b && c <= 0x7d))
    uch = c;
  else if (c == 0x24)
    uch = 0x00a4;
  else if (c == 0x2c)
    uch = 0x060c;
  else if (c == 0x3b)
    uch = 0x061b;
  else if (c == 0x3f)
    uch = 0x061f;
  else if ((c >= 0x41 && c <= 0x5a)
           || (c >= 0x60 && c <= 0x72))
    uch = c + 0x5e0;
  else if (c == 0x7e)
    uch = 0x203e;
  
  out += uch;
}

void
arabic::invoke (code_element elt, std::string &out)
{
  switch (elt) {
  case ELEMENT_G0:
    out += "\x1b\x28\x6b";
    break;
  case ELEMENT_G1:
    out += "\x1b\x29\x6b";
    break;
  case ELEMENT_G2:
    out += "\x1b\x2a\x6b";
    break;
  case ELEMENT_G3:
    out += "\x1b\x2b\x6b";
    break;
  }
}

bool
arabic::encode (const char16_t *&pcs,
                const char16_t *pcsend,
                unsigned char base,
                std::string &out)
{
  if (pcsend > pcs + 1)
    return false;

  char16_t c = *pcs;
  unsigned char chout;

  if ((c >= 0x20 && c <= 0x23)
      || (c >= 0x25 && c <= 0x2b)
      || (c >= 0x2d && c <= 0x3a)
      || (c >= 0x3c && c <= 0x3e)
      || c == 0x40
      || (c >= 0x5b && c <= 0x5f)
      || (c >= 0x7b && c <= 0x7d))
    chout = c;
  else if (c == 0x00a4)
    chout = 0x24;
  else if (c == 0x060c)
    chout = 0x2c;
  else if (c == 0x061b)
    chout = 0x3b;
  else if (c == 0x061f)
    chout = 0x3f;
  else if ((c >= 0x621 && c <= 0x63a)
           || (c >= 0x640 && c <= 0x652))
    chout = c - 0x5e0;
  else if (c == 0x203e)
    chout = 0x7e;
  else
    return false;

  out += (char)(chout + base);

  return true;
}
