// 103 - Teletex Supplementary Set of Graphic Characters CCITT Rec. T.61

#include "../codesets.h"

using namespace iso2022::builtin;

/* This set has some non-spacing characters in it :-(
   These are somewhat difficult to deal with, because they are
   used with characters FROM OTHER SETS! */

#define NE 0xfffd

static const char16_t teletex_supp_map[128] = {
      NE,     NE,     NE,     NE,     NE,     NE,     NE,     NE,
      NE,     NE,     NE,     NE,     NE,     NE,     NE,     NE,

      NE,     NE,     NE,     NE,     NE,     NE,     NE,     NE,
      NE,     NE,     NE,     NE,     NE,     NE,     NE,     NE,

      NE, 0x00a1, 0x00a2, 0x00a3, 0x0024, 0x00a5, 0x0023, 0x00a7,
  0x00a4,     NE,     NE, 0x00ab,     NE,     NE,     NE,     NE,

  0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00d7, 0x00b5, 0x00b6, 0x00b7,
  0x00f7,     NE,     NE, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,

      // ###FIXME: We should support the combining characters here
      NE,     NE,     NE,     NE,     NE,     NE,     NE,     NE,
      NE,     NE,     NE,     NE,     NE,     NE,     NE,     NE,

      NE,     NE,     NE,     NE,     NE,     NE,     NE,     NE,
      NE,     NE,     NE,     NE,     NE,     NE,     NE,     NE,

  0x2126, 0x00c6, 0x00d0, 0x00aa, 0x0126,     NE, 0x0132, 0x013f,
  0x0141, 0x00d8, 0x0152, 0x00ba, 0x00de, 0x0166, 0x014a, 0x0149,

  0x0138, 0x00e6, 0x0111, 0x00f0, 0x0127, 0x0131, 0x0133, 0x0140,
  0x0142, 0x00f8, 0x0153, 0x00df, 0x00fe, 0x0167, 0x014b,     NE,
};

void
teletex_supp::decode (unsigned char c, std::u16string &out)
{
  out += teletex_supp_map[c];
}

void
teletex_supp::invoke (code_element elt, std::string &out)
{
  switch (elt) {
  case ELEMENT_G0:
    out += "\x1b\x28\x76";
    break;
  case ELEMENT_G1:
    out += "\x1b\x29\x76";
    break;
  case ELEMENT_G2:
    out += "\x1b\x2a\x76";
    break;
  case ELEMENT_G3:
    out += "\x1b\x2b\x76";
    break;
  }
}

bool
teletex_supp::encode (const char16_t *&pcs,
                      const char16_t *pcsend,
                      unsigned char base,
                      std::string &out)
{
  if (pcsend > pcs + 1)
    return false;

  unsigned char chout;

  switch (*pcs) {
  case 0x00a1: chout = 0x21; break;
  case 0x00a2: chout = 0x22; break;
  case 0x00a3: chout = 0x23; break;
  case 0x0024: chout = 0x24; break;
  case 0x00a5: chout = 0x25; break;
  case 0x0023: chout = 0x26; break;
  case 0x00a7: chout = 0x27; break;
  case 0x00a4: chout = 0x28; break;
  case 0x00ab: chout = 0x2b; break;

  case 0x00b0: chout = 0x30; break;
  case 0x00b1: chout = 0x31; break;
  case 0x00b2: chout = 0x32; break;
  case 0x00b3: chout = 0x33; break;
  case 0x00d7: chout = 0x34; break;
  case 0x00b5: chout = 0x35; break;
  case 0x00b6: chout = 0x36; break;
  case 0x00b7: chout = 0x37; break;
  case 0x00f7: chout = 0x38; break;
  case 0x00bb: chout = 0x3b; break;
  case 0x00bc: chout = 0x3c; break;
  case 0x00bd: chout = 0x3d; break;
  case 0x00be: chout = 0x3e; break;
  case 0x00bf: chout = 0x3f; break;

  case 0x2126: chout = 0x60; break;
  case 0x00c6: chout = 0x61; break;
  case 0x00d0: chout = 0x62; break;
  case 0x00aa: chout = 0x63; break;
  case 0x0126: chout = 0x64; break;
  case 0x0132: chout = 0x66; break;
  case 0x013f: chout = 0x67; break;
  case 0x0141: chout = 0x68; break;
  case 0x00d8: chout = 0x69; break;
  case 0x0152: chout = 0x6a; break;
  case 0x00ba: chout = 0x6b; break;
  case 0x00de: chout = 0x6c; break;
  case 0x0166: chout = 0x6d; break;
  case 0x014a: chout = 0x6e; break;
  case 0x0149: chout = 0x6f; break;

  case 0x0138: chout = 0x70; break;
  case 0x00e6: chout = 0x71; break;
  case 0x0111: chout = 0x72; break;
  case 0x00f0: chout = 0x73; break;
  case 0x0127: chout = 0x74; break; 
  case 0x0131: chout = 0x75; break;
  case 0x0133: chout = 0x76; break;
  case 0x0140: chout = 0x77; break;
  case 0x0142: chout = 0x78; break;
  case 0x00f8: chout = 0x79; break;
  case 0x0153: chout = 0x7a; break;
  case 0x00df: chout = 0x7b; break;
  case 0x00fe: chout = 0x7c; break;
  case 0x0167: chout = 0x7d; break;
  case 0x014b: chout = 0x7e; break;

  default:
    return false;
  }

  out += (char)(chout + base);
  return true;
}
