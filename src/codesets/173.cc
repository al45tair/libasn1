// 072 - Third Supplementary Set of Mosaic Characters
// 173 - Third Supplementary Set of Mosaic Characters

#include "../codesets.h"

using namespace iso2022::builtin;

// Note: Quite a few characters in this set have no Unicode equivalent

#define NE 0xfffd

static const char16_t vt173_map[128] = {
      NE,     NE,     NE,     NE,     NE,     NE,     NE,     NE,
      NE,     NE,     NE,     NE,     NE,     NE,     NE,     NE,

      NE,     NE,     NE,     NE,     NE,     NE,     NE,     NE,
      NE,     NE,     NE,     NE,     NE,     NE,     NE,     NE,

      NE, 0x2528, 0x2512, 0x2511, 0x251a, 0x2519, 0x2520, 0x2538,
  0x2530, 0x2516, 0x2515, 0x250d, 0x250e, 0x2542,     NE, 0x258c,

  0x2503, 0x2501, 0x250f, 0x2513, 0x2517, 0x251b, 0x2523, 0x252b,
  0x2533, 0x253b, 0x254b, 0x2580, 0x2584, 0x2588, 0x25aa, 0x2590,

  0x2537, 0x252f, 0x251d, 0x2525,     NE,     NE,     NE,     NE,
      NE,     NE,     NE,     NE, 0x253f, 0x2022, 0x25cf, 0x25cb,

  0x2502, 0x2500, 0x250c, 0x2510, 0x2514, 0x2518, 0x251c, 0x2524,
  0x252c, 0x2534, 0x253c, 0x2192, 0x2190, 0x2191, 0x2193, 0x2591,

      NE,     NE,     NE,     NE,     NE,     NE,     NE,     NE,
      NE,     NE,     NE,     NE,     NE,     NE, 0x2592, 0x2593,

      NE,     NE,     NE,     NE,     NE,     NE,     NE,     NE,
      NE,     NE,     NE,     NE,     NE,     NE,     NE,     NE,
};

void
videotex_173::decode (unsigned char c, std::u16string &out)
{
  out += vt173_map[c];
}

void
videotex_173::invoke (code_element elt, std::string &out)
{
  // Since we use 173 rather than 72, we need to include ESC 0x26 0x40
  switch (elt) {
  case ELEMENT_G0: out += "\x1b\x26\x40\x1b\x28\x64"; break;
  case ELEMENT_G1: out += "\x1b\x26\x40\x1b\x29\x64"; break;
  case ELEMENT_G2: out += "\x1b\x26\x40\x1b\x2a\x64"; break;
  case ELEMENT_G3: out += "\x1b\x26\x40\x1b\x2b\x64"; break;
  }
}

bool
videotex_173::encode (const char16_t *&pcs,
                      const char16_t *pcsend,
                      unsigned char base,
                      std::string  &out)
{
  if (pcsend != pcs + 1)
    return false;

  unsigned char chout;

  switch (*pcs) {
  case 0x2528: chout = 0x21; break;
  case 0x2512: chout = 0x22; break;
  case 0x2511: chout = 0x23; break;
  case 0x251a: chout = 0x24; break;
  case 0x2519: chout = 0x25; break;
  case 0x2520: chout = 0x26; break;
  case 0x2538: chout = 0x27; break;
  case 0x2530: chout = 0x28; break;
  case 0x2516: chout = 0x29; break;
  case 0x2515: chout = 0x2a; break;
  case 0x250d: chout = 0x2b; break;
  case 0x250e: chout = 0x2c; break;
  case 0x2542: chout = 0x2d; break;
  case 0x258c: chout = 0x2f; break;

  case 0x2503: chout = 0x30; break;
  case 0x2501: chout = 0x31; break;
  case 0x250f: chout = 0x32; break;
  case 0x2513: chout = 0x33; break;
  case 0x2517: chout = 0x34; break;
  case 0x251b: chout = 0x35; break;
  case 0x2523: chout = 0x36; break;
  case 0x252b: chout = 0x37; break;
  case 0x2533: chout = 0x38; break;
  case 0x253b: chout = 0x39; break;
  case 0x254b: chout = 0x3a; break;
  case 0x2580: chout = 0x3b; break;
  case 0x2584: chout = 0x3c; break;
  case 0x2588: chout = 0x3d; break;
  case 0x25aa: chout = 0x3e; break;
  case 0x2590: chout = 0x3f; break;

  case 0x2537: chout = 0x40; break;
  case 0x252f: chout = 0x41; break;
  case 0x251d: chout = 0x42; break;
  case 0x2525: chout = 0x43; break;
  case 0x253f: chout = 0x4c; break;
  case 0x2022: chout = 0x4d; break;
  case 0x25cf: chout = 0x4e; break;
  case 0x25cb: chout = 0x4f; break;

  case 0x2502: chout = 0x50; break;
  case 0x2500: chout = 0x51; break;
  case 0x250c: chout = 0x52; break;
  case 0x2510: chout = 0x53; break;
  case 0x2514: chout = 0x54; break;
  case 0x2518: chout = 0x55; break;
  case 0x251c: chout = 0x56; break;
  case 0x2524: chout = 0x57; break;
  case 0x252c: chout = 0x58; break;
  case 0x2534: chout = 0x59; break;
  case 0x253c: chout = 0x5a; break;
  case 0x2192: chout = 0x5b; break;
  case 0x2190: chout = 0x5c; break;
  case 0x2191: chout = 0x5d; break;
  case 0x2193: chout = 0x5e; break;
  case 0x2591: chout = 0x5f; break;

  case 0x2592: chout = 0x6e; break;
  case 0x2593: chout = 0x6f; break;

  default:
    return false;
  }

  out += (char)(chout + base);

  return true;
}
