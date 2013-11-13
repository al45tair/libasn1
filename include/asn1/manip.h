/* Emacs, this is -*-C++-*- */

#ifndef ASN1_MANIP_H_
#define ASN1_MANIP_H_

#include "base.h"
#include "Tag.h"
#include "BERDecoder.h"
#include "DEREncoder.h"

BEGIN_ASN1_NS

class tag {
private:
  Tag _t;
  PrimitiveOrConstructed _c;
public:
  tag(const tag &o) : _t(o._t), _c(o._c) {}
  tag(Tag t, PrimitiveOrConstructed c = PRIMITIVE) : _t(t), _c(c) {}

  friend DEREncoder &operator<< (DEREncoder &e, tag t);
  friend BERDecoder &operator>> (BERDecoder &d, tag t);
};

/* Allows you to override the next tag, e.g.

     e << asn1::tag(tFoo) << 3;

   This will work for sequences and sets too (and overrides just the sequence/
   set tag), and it works on input as well as output. */
inline DEREncoder &operator<< (DEREncoder &e, tag t) {
  e.overrideNextTag (t._t, t._c);
  return e;
}
inline BERDecoder &operator>> (BERDecoder &e, tag t) {
  d.overrideNextTag (t._t, t._c);
  return d;
}

class instance_of {
private:
  OID _o;
public:
  instance_of(const instance_of &o) : _o(o._o) {}
  instance_of(instance_of &&o) : _o(std::move(o._o)) {}
  explicit instance_of(const OID &o) : _o(o) {}

  const OID &oid() const { return _o; }

  friend DEREncoder &operator<< (DEREncoder &e, instance_of i);
  friend BERDecoder &operator>> (BERDecoder &d, instance_of &i);
};

// e << asn1::instance_of(my_oid) << <value encoding...>
inline DEREncoder &operator<< (DEREncoder &e, instance_of i) {
  e.encodeTag (tInstanceOf);
  e.pushState (DEREncoder::SEQUENCE);
  return e << i._o;
}

/* asn1::instance_of i;

   d >> i;

   std::cout << i.oid(); */
inline BERDecoder &operator>> (BERDecoder &d, instance_of &i) {
  d.expectTag (tInstanceOf);
  uint32 len = d.decodeLength();
  d.pushState (false, len);
  d >> i._o;
  return d;
}

template <class Enum>
class enumerated {
private:
  Enum _e;
public:
  enumerated(const enumerated<Enum> &o) : _i(o._e) {}
  enumerated(enumerated<Enum> &&o) : _e(std::move(o._e)) {}
  explicit enumerated(Enum e) : _e(e) {}

  operator Enum() const { return _e; }
  enumerated &operator=(Enum e) { _e = e; }

  friend DEREncoder &operator<< (DEREncoder &e, enumerated<Enum> en);
  friend BERDecoder &operator>> (BERDecoder &d, enumerated<Enum> &en);
};

/* Sadly there's no way to have a base class for enums, or for a template to
   select specifically enums.  As a result, in order for us to send an
   ASN.1 enumerated type, we need to use asn1::enumerated to wrap the enum.
   If you try to encode or decode an enum directly, it will encode as an
   integer :-( :-( */

/* enum foo { FOO=0, BAR=1, FOOBAR=2 };
   e << asn1::enumerated<foo>(FOOBAR); */
template <class Enum>
DEREncoder &operator<< (DEREncoder &e, enumerated<Enum> en) {
  e.overrideNextTag (tEnumerated, PRIMITIVE);
  return e << en._e;
}

/* enum foo { FOO=0, BAR=1, FOOBAR=2 };
   asn1::enumerated<foo> e;

   d >> e; */
template <class Enum>
BERDecoder &operator>> (BERDecoder &d, enumerated<Enum> &en)
{
  e.overrideNextTag (tEnumerated, PRIMITIVE);
  return d >> en._e;
}

END_ASN1_NS

#endif /* ASN1_MANIP_H_ */
