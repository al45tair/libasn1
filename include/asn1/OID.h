/* Emacs, this is -*-C++-*- */

#ifndef ASN1_OID_H_
#define ASN1_OID_H_

#include "base.h"

BEGIN_ASN1_NS

class RelativeOID : public std::vector<uint32>
{
public:
  RelativeOID(const RelativeOID &o) : std::vector<uint32>(o) {}
  RelativeOID(RelativeOID &&o) : std::vector<uint32>(o) {}
  RelativeOID(std::initializer_list<uint32> il) : std::vector<uint32>(il) { }
};

class OID : public std::vector<uint32>
{
public:
  OID(const OID &o) : std::vector<uint32>(o) {}
  OID(OID &&o) : std::vector<uint32>(o) {}
  OID(const OID &o, const RelativeOID &ro) : std::vector<uint32>(o) {
    insert (end(), ro.begin(), ro.end());
  }
  OID(std::initializer_list<uint32> il) : std::vector<uint32>(il) { }
};

inline std::ostream &operator<<(std::ostream &os, const OID &o) {
  if (!o.size())
    os << "<empty>";
  else {
    os << o[0];
    for (auto i = o.begin() + 1; i < o.end(); ++i)
      os << '.' << *i;
  }
  return os;
}

inline std::ostream &operator<<(std::ostream &os, const RelativeOID &o) {
  if (!o.size())
    os << "<empty>";
  else {
    os << "<rel>." << o[0];
    for (auto i = o.begin() + 1; i < o.end(); ++i)
      os << '.' << *i;
  }
  return os;
}

END_ASN1_NS

#endif /* ASN1_OID_H_ */
