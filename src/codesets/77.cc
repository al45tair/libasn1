// 077 - C1 Control Set of ISO 6429-1983

#include "../codesets.h"

using namespace iso2022::builtin;

int
standard_c1::decode (unsigned char c) const
{
  return c;
}

int
standard_c1::encode (unsigned char c) const
{
  return c;
}
