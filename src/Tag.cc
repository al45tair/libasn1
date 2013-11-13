#include <asn1/Tag.h>

std::ostream &
asn1::operator<<(std::ostream &os, Tag t)
{
  static const char *classes[] = { "[UNIVERSAL ",
                                   "[APPLICATION ",
                                   "[CONTEXT SPECIFIC ",
                                   "[PRIVATE " };

  return os << classes[t.tagClass] << t.number << ']';
}
