// 073 - Attribute Control Set for Videotex

#include "../codesets.h"

using namespace iso2022::builtin;

// We actually can't encode or decode any of these things!

int
videotex_attr::decode (unsigned char c) const
{
  (void)c;
  return -1;
}

int
videotex_attr::encode (unsigned char c) const
{
  (void)c;
  return -1;
}
