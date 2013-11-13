// 001 - C0 Set of ISO 646

#include "../codesets.h"

using namespace iso2022::builtin;

int
standard_c0::decode (unsigned char c) const
{
  return c;
}

int
standard_c0::encode (unsigned char c) const
{
  return c;
}
