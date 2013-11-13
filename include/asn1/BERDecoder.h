/* Emacs, this is -*-C++-*- */

#ifndef ASN1_BERDECODER_H_
#define ASN1_BERDECODER_H_

#include "base.h"
#include "Tag.h"
#include "BitString.h"
#include "OID.h"
#include "strings.h"

#include <vector>
#include <list>
#include <deque>
#include <map>
#include <set>

BEGIN_ASN1_NS

class BERDecoder
{
private:
  const octet *_ptr;
  const octet *_end;

private:
  struct State {
    const octet *end;

    State(const octet *e) : end(e) {}
    State(State &&other) : end(other.end) {}
    State(const State &other) : end(other.end) {}

    State &operator=(const State &other) {
      end = other.end;
      return *this;
    }
    State &operator=(State &&other) {
      end = other.end;
      return *this;
    }
  };

  State              *_state;
  std::vector<State>  _stack;

  bool                _override_next_tag;
  Tag                 _next_tag;
  bool                _next_tag_constructed;

public:
  BERDecoder (const octet *data, size_t len)
    : _ptr(data), _end(data + len), _stack(1, State(data + len)) { 
    _state = &_stack.back();
  }

  bool inIndefinite() { 
    return !_state->end;
  }

  bool atEnd() {
    // If we're in an indefinite mode, we also report atEnd() an an EOC
    return _ptr >= _end || !_state->end && !*_ptr;
  }

  octet getOctet() {
    if (_ptr >= _end)
      throw std::runtime_error("out of bounds");

    return *_ptr++;
  }

  uint16 get16() {
    if (_ptr + 2 > _end)
      throw std::runtime_error("out of bounds");

    uint16 ret = *(uint16 *)_ptr;
    _ptr += 2;
    return machine::from_be(ret);
  }

  uint32 get32() {
    if (_ptr + 4 > _end)
      throw std::runtime_error("out of bounds");

    uint32 ret = *(uint32 *)_ptr;
    _ptr += 4;
    return machine::from_be(ret);
  }

  uint32 get64() {
    if (_ptr + 8 > _end)
      throw std::runtime_error("out of bounds");

    uint64 ret = *(uint64 *)_ptr;
    _ptr += 8;
    return machine::from_be(ret);
  }

  uint32 getTBF() {
    uint32 result = 0;
    unsigned count = 0;

    do {
      if (++count > 4)
        throw std::runtime_error("bad TBF value");
      o = getOctet();
      result = (result << 7) | (o & 0x7f);
    } while (o & 0x80);

    return result;
  }

  const octet *getOctets(size_t n) {
    if (_ptr + n > _end)
      throw std::runtime_error("out of bounds");

    const octet *ret = _ptr;
    _ptr += n;
    return ret;
  }

  Tag decodeTag(PrimitiveOrConstructed &c) {
    octet t = getOctet();
    TagClass tc = (TagClass)(t >> 6);
    
    c = (t & 0x20) ? CONSTRUCTED : PRIMITIVE;
    
    if ((t & 0x1f) == 0x1f) {
      return Tag(tc, getTBF());
    } else {
      return Tag(tc, t & 0x1f);
    }
  }

  Tag peekTag(PrimitiveOrConstructed &c) {
    const octet *savedPtr = _ptr;
    Tag t = decodeTag(c);
    _ptr = savedPtr;
    return t;
  }

  uint32 decodeLength() {
    octet o = getOctet();

    if (o <= 0x7f)
      return o;

    unsigned count = o & 0x7f;

    if (count > 4 || count < 1)
      throw std::runtime_error("bad length value");

    if (count == 4)
      return get32();
    else if (count == 3)
      return (getOctet() << 24) | get16();
    else if (count == 2)
      return get16();
    else
      return getOctet();
  }

  uint32 decodeLengthOrIndefinite(bool &indefinite) {
    octet o = getOctet();

    if (o == 0x80) {
      indefinite = true;
      return 0;
    }

    if (o < 0x7f)
      return o;

    unsigned count = o & 0x7f;

    if (count > 4 || count < 1)
      throw std::runtime_error("bad length value");

    if (count == 4)
      return get32();
    else if (count == 3)
      return (getOctet() << 24) | get16();
    else if (count == 2)
      return get16();
    else
      return getOctet();
  }      

  void overrideNextTag(const Tag t, PrimitiveOrConstructed c)
  {
    if (!_override_next_tag) {
      _override_next_tag = true;
      _next_tag = t;
      _next_tag_constructed = c;
    }
  }

  void expectTag(Tag t, PrimitiveOrConstructed pc = PRIMITIVE) {
    PrimitiveOrConstructed c;
    Tag rt = decodeTag (c);

    if (_override_next_tag) {
      _override_next_tag = false;
      t = _next_tag;
      pc = _next_tag_constructed;
    }

    if (rt != t || c != pc)
      throw std::runtime_error("unexpected tag");
  }

  void expectEndOfContents() {
    octet t = getOctet();
    if (t != 0)
      throw std::runtime_error("expected End of Contents tag");
    octet l = getOctet();
    if (l != 0)
      throw std::runtime_error("bad length on End of Contents tag");
  }

  void pushState(bool indefinite=true, uint32 len=0) {
    if (!indefinite && _ptr + len > _end)
      throw std::runtime_error("out of bounds");

    _stack.push_back(BEREncoder::State(indefinite ? NULL : _ptr + len));
    _state = &_stack.back();
    if (_state->end)
      _end = _state->end;
  }
  void popState() {
    _stack.pop_back();
    _state = &_stack.back();
    if (_state->end)
      _end = _state->end;
  }
};

inline BERDecoder &operator>> (BERDecoder &d, BERDecoder &(*pf)(BERDecoder &)) {
  return pf(d);
}

inline BERDecoder &operator>> (BERDecoder &d, bool &b) {
  d.expectTag (tBoolean);
  if (d.decodeLength() != 1)
    throw std::runtime_error("incorrect length for boolean");
  b = d.getOctet() ? true : false;

  return d;
}

inline BERDecoder &operator>> (BERDecoder &d, int32 &i) {
  d.expectTag (tInteger);
  uint32 len = d.decodeLength();

  if (!len || len > 4)
    throw std::runtime_error("integer outside range for int32");

  octet o = d.getOctet();
  i = 0;
  if (o & 0x80) {
    for (unsigned n = len; n < 4; ++n)
      i = (i << 8) | 0xff;
  }
  i = (i << 8) | o;
  for (unsigned n = 1; n < len; ++n)
    i = (i << 8) | d.getOctet();

  return d;
}

inline BERDecoder &operator>> (BERDecoder &d, uint32 &u) {
  d.expectTag (tInteger);
  uint32 len = d.decodeLength();

  if (!len || len > 5)
    throw std::runtime_error("integer outside range for uint32");

  octet o = d.getOctet();

  if ((len == 5 && o) || (o & 0x80))
    throw std::runtime_error("integer outside range for uint32");

  u = o;
  for (unsigned n = 1; n < len; ++n)
    u = (u << 8) | d.getOctet();

  return d;
}

inline BERDecoder &operator>> (BERDecoder &d, int64 &i) {
  d.expectTag (tInteger);
  uint32 len = d.decodeLength();

  if (!len || len > 8)
    throw std::runtime_error("integer outside range for int64");

  octet o = d.getOctet();
  i = 0;
  if (o & 0x80) {
    for (unsigned n = len; n < 8; ++n)
      i = (i << 8) | 0xff;
  }
  i = (i << 8) | o;
  for (unsigned n = 1; n < len; ++n)
    i = (i << 8) | d.getOctet();

  return d;
}

inline BERDecoder &operator>> (BERDecoder &d, uint64 &u) {
  d.expectTag (tInteger);
  uint32 len = d.decodeLength();

  if (!len || len > 9)
    throw std::runtime_error("integer outside range for uint64");

  octet o = d.getOctet();

  if ((len == 9 && o) || (o & 0x80))
    throw std::runtime_error("integer outside range for uint64");

  u = o;
  for (unsigned n = 1; n < len; ++n)
    u = (u << 8) | d.getOctet();

  return d;
}

inline BERDecoder &operator>> (BERDecoder &d, double &r) {
  octet o;
  union {
    uint64 u;
    double d;
  } un;
  
  d.expectTag (tReal);
  uint32 len = d.decodeLength();
  
  /* 8.5.2 If the real value is the value plus zero, there shall be no
     contents octets in the encoding */
  if (!len) {
    r = 0;
    return d;
  }

  // 8.5.9
  if (len == 1) {
    o = d.getOctet();

    if ((o & 0xc0) == 0x40) {
      switch (o) {
      case 0x40:
        // 8.5.9 PLUS-INFINITY
        un.u = 0x7ff0000000000000;
        r = un.d;
        break;
      case 0x41:
        // 8.5.9 MINUS-INFINITY
        un.u = 0xfff0000000000000;
        r = un.d;
        break;
      case 0x42:
        // 8.5.9 NOT-A-NUMBER
        un.u = 0x7ff0000000000001;
        r = un.d;
        break;
      case 0x43:
        // 8.5.9 minus zero
        un.u = 0x8000000000000000;
        r = un.d;
        break;
      default:
        throw std::runtime_error("unknown special real value");
      }
      return d;
    }

    throw std::runtime_error("invalid real encoding");
  }

  // The first contents octet tells us more
  o = d.getOctet();

  if ((o & 0xc0) == 0x00)
    throw std::runtime_error("attempt to read base-10 real into IEEE double");

  if ((o & 0xc0) == 0x40)
    throw std::runtime_error("length of special real value must be 1");

  unsigned base;

  switch (o & 0x30) {
  case 0: base = 2; break;
  case 1: base = 8; break;
  case 2: base = 16; break;
  default:
    throw std::runtime_error("unknown base for real number");
  }

  unsigned factor = (o >> 2) & 0x03;
  bool sign = o & 0x40;
  int32 exponent = 0;

  switch (o & 0x03) {
  case 0x00:
    exponent = d.getOctet();
    if (exponent & 0x80)
      exponent |= 0xffffff00;
    len -= 2;
    break;
  case 0x01:
    exponent = d.getOctet();
    exponent = (exponent << 8) | d.getOctet();
    if (exponent & 0x8000)
      exponent |= 0xffff0000;
    len -= 3;
    break;
  case 0x02:
    exponent = d.getOctet();
    exponent = (exponent << 8) | d.getOctet();
    exponent = (exponent << 8) | d.getOctet();
    if (exponent & 0x800000)
      exponent |= 0xff000000;
    len -= 4;
    break;
  case 0x03:
    {
      unsigned elen = d.getOctet();

      if (!elen)
        throw std::runtime_error("zero-length exponent(!) in real number");

      if (elen > 4)
        throw std::runtime_error("exponent too large in real number");

      unsigned sign = 0x80;

      for (unsigned n = 0; n < elen; ++n) {
        exponent = (exponent << 8) | d.getOctet();
        sign <<= 8;
      }

      if (exponent & sign) {
        for (unsigned n = elen; n < 4; ++n)
          exponent |= 0xff << (8 * n);
      }

      len -= elen + 1;
    }
    break;
  }

  if (!len)
    throw std::runtime_error("no mantissa octets in real number");

  // Convert the exponent to base 2
  if (base == 8)
    exponent *= 3;
  else if (base == 16)
    exponent *= 4;

  // Adjust it
  exponent += factor;

  // Grab the mantissa
  uint64 u;

  switch (len) {
  case 1:
    u = d.getOctet();
    break;
  case 2:
    u = d.get16();
    break;
  case 3:
    u = d.get16() << 8 | d.getOctet();
    break;
  case 4:
    u = d.get32() << 32;
    break;
  case 5:
    u = d.get32() << 8 | d.getOctet();
    break;
  case 6:
    u = d.get32() << 16 | d.get16();
    break;
  case 7:
    u = d.get32() << 24 | d.get16() << 8 | d.getOctet();
    break;
  case 8:
  default:
    u = d.get64();

    // Skip any remaining octets
    d.getOctets(len - 8);
    break;
  }

  // Align the mantissa
  unsigned lz = machine::clz(u);

  if (lz > 11) {
    unsigned shift = lz - 11;

    exponent -= shift;

    u <<= shift;
  } else if (lz < 11) {
    unsigned shift = 11 - lz;

    exponent += shift;

    u >>= shift;
  }

  if (exponent < -1022) {
    // Need to turn this into a subnormal number
    unsigned shift = -1022 - exponent;
    if (shift > 52)
      u = 0;
    else
      u >>= shift;
    exponent = 0;
  } else if (exponent > 1023) {
    // The exponent is too large, so we map to an infinity
    un.u = 0x7ff0000000000000;
    if (sign)
      un.u |= 0x8000000000000000;
    r = un.d;
    return d;
  } else {
    exponent += 1023;
  }

  // Strip the implied 1, if present
  u &= 0x000fffffffffffff;
  u |= exponent << 52;
  if (sign)
    u |= 0x8000000000000000;

  un.u = u;
  r = un.d;

  return d;
}

template <class A>
BERDecoder &operator>> (BERDecoder &d, std::vector<octet, A> &v) {
  d.expectTag (tOctetString);
  uint32 len = d.decodeLength();
  octet *ptr = d.getOctets (len);
  v.insert (v.end(), ptr, ptr + len);
  return d;
}

template <class A>
BERDecoder &operator>> (BERDecoder &d, const BitString<A> &v) {
  d.expectTag (tBitString);
  uint32 len = d.decodeLength();
  octet ignored = d.getOctet();
  v.assign (d.getOctets(len), len * 8 - ignored);
  return d;
}

inline BERDecoder &null(BERDecoder &d) {
  d.expectTag (tNull);
  uint32 len = d.decodeLength();
  if (len != 0)
    throw std::runtime_error("expected zero length for a Null");
  return d;
}

/* BERDecoder d;
   ASN1::T61String name;
   bool flag;

   d >> ASN1::sequence >> name >> flag >> ASN1::end;

   d >> ASN1::set >> name >> flag >> ASN1::end;

   d >> ASN1::set;
   while (!d.atEnd()) {
     d >> name;
   }
   d >> ASN1::end;
*/
inline BERDecoder &sequence(BERDecoder &d) {
  d.expectTag (tSequence, CONSTRUCTED);
  bool indefinite = false;
  uint32 len = d.decodeLengthOrIndefinite (indefinite);

  d.pushState (indefinite, len);
  return d;
}

inline BERDecoder &set(BERDecoder &d) {
  d.expectTag (tSet, CONSTRUCTED);
  bool indefinite, false;
  uint32 len = d.decodeLengthOrIndefinite (indefinite);

  d.pushState (indefinite, len);
  return d;
}

inline BERDecoder &end(BERDecoder &d) {
  if (!d.atEnd())
    throw std::runtime_error("expected end, got more octets");
  if (d.inIndefinite())
    d.expectEndOfContents();
  d.popState ();
  return d;
}

/* BERDecoder d;
   std::vector<int> v;

   d >> v;

   is equivalent do

   d >> ASN1::sequence >> v[0] >> v[1] >> ... >> v[n] >> ASN1::end;

   Similarly for std::list and std::deque.
*/
template <class T, class A=std::allocator<T> >
BERDecoder &operator>> (BERDecoder &d, std::vector<T, A>  &v) {
  d >> ASN1::sequence;
  while (!d.atEnd()) {
    T val;
    d >> val;
    v.push_back(val);
  }
  d >> ASN1::end;
  return d;
}
template <class T, class A=std::allocator<T> >
BERDecoder &operator>> (BERDecoder &d, std::list<T, A>  &v) {
  d >> ASN1::sequence;
  while (!d.atEnd()) {
    T val;
    d >> val;
    v.push_back(val);
  }
  d >> ASN1::end;
  return d;
}
template <class T, class A=std::allocator<T> >
BERDecoder &operator>> (BERDecoder &d, std::deque<T, A>  &v) {
  d >> ASN1::sequence;
  while (!d.atEnd()) {
    T val;
    d >> val;
    v.push_back(val);
  }
  d >> ASN1::end;
  return d;
}

/* BERDecoder d;
   std::set<int> s;

   d >> s;

   is equivalent to

   d >> ASN1::set >> s[0] >> s[1] >> ... >> s[n] >> ASN1::end
*/
template <class T, class A=std::allocator<T> >
BERDecoder &operator>> (BERDecoder &d, std::set<T, A>  &v) {
  d >> ASN1::sequence;
  while (!d.atEnd()) {
    T val;
    d >> val;
    s.insert(val);
  }
  d >> ASN1::end;
  return d;
}

/* BERDecoder d;
   std::map<int, int> m;

   d >> m;

   is equivalent to

   d >> ASN1::set
       >> ASN1::sequence >> key[0] >> value[0] >> ASN1:end
       >> ASN1::sequence >> key[1] >> value[1] >> ASN1:end
       >> ...
       >> ASN1::sequence >> key[n] >> value[n] >> ASN1:end
     >> ASN1::end;

   Similarly for std::multimap.
*/
template <class Key, class T, class Compare=std::less<Key>,
          class A=std::allocator<std::pair<const Key, T> > >
BERDecoder &operator>> (BERDecoder &d, std::map<Key, T, Compare, A> &m)
{
  d >> ASN1::set;
  while (!d.atEnd()) {
    Key k;
    T v;
    d >> ASN1::sequence >> k >> v >> ASN1::end;
    m.emplace(k, v);
  }
  d >> ASN1::end;
  return d;
}
template <class Key, class T, class Compare=std::less<Key>,
          class A=std::allocator<std::pair<const Key, T> > >
BERDecoder &operator>> (BERDecoder &d, std::multimap<Key, T, Compare, A> &m)
{
  d >> ASN1::set;
  while (!d.atEnd()) {
    Key k;
    T v;
    d >> ASN1::sequence >> k >> v >> ASN1::end;
    m.emplace(k, v);
  }
  d >> ASN1::end;
  return d;
}

inline BERDecoder &operator>> (BERDecoder &d, OID &o)
{
  d.expectTag (tOID);
  uint32 len = d.decodeLength();

  d.pushState (false, len);

  unsigned subid = d.getTBF();

  o.clear();
  o.push_back(subid / 40);
  o.push_back(subid % 40);

  while (!d.atEnd())
    o.push_back(d.getTBF());

  d.popState();

  return d;
}

inline BERDecoder &operator>> (BERDecoder &d, RelativeOID &o)
{
  d.expectTag (tRelativeOID);
  uint32 len = d.decodeLength();

  d.pushState (false, len);

  o.clear();
  while (!d.atEnd())
    o.push_back(d.getTBF());

  d.popState();

  return d;
}

// String types
inline BERDecoder &operator>> (BERDecoder &d, const BMPString &bmp) {
  d.expectTag (tBMPString);
  uint32 len = d.decodeLength();

  if (len & 1)
    throw std::runtime_error("BMP string must have even number of octets");

  len >>= 1;

  bmp.clear();
  while (len--)
    bmp.push_back (d.get16());
  
  return d;
}

inline BERDecoder &operator>> (BERDecoder &d, const UniversalString &us) {
  d.expectTag (tUniversalString);
  uint32 len = d.decodeLength();

  if (len & 3)
    throw std::runtime_error("Universal string must have a multiple of four octets");

  len >>= 2;

  us.clear();
  while (len--)
    us.push_back (d.get32());

  return d;
}

inline BERDecoder &operator>> (BERDecoder &d, const GeneralString &gs) {
  d.expectTag (tGeneralString);
  uint32 len = d.decodeLength();

  gs.assign (d.getOctets(len), len);

  return d;
}

inline BERDecoder &operator>> (BERDecoder &d, const GraphicString &gs) {
  d.expectTag (tGraphicString);
  uint32 len = d.decodeLength();

  gs.assign (d.getOctets(len), len);

  return d;
}

inline BERDecoder &operator>> (BERDecoder &d, const IA5String &ia5) {
  d.expectTag (tIA5String);
  uint32 len = d.decodeLength();

  ia5.assign (d.getOctets(len), len);

  return d;
}

inline BERDecoder &operator>> (BERDecoder &d, const NumericString &ns) {
  d.expectTag (tNumericString);
  uint32 len = d.decodeLength();

  ns.assign (d.getOctets(len), len);

  return d;
}

inline BERDecoder &operator>> (BERDecoder &d, const PrintableString &ps) {
  d.expectTag (tPrintableString);
  uint32 len = d.decodeLength();

  ps.assign (d.getOctets(len), len);

  return d;
}

inline BERDecoder &operator>> (BERDecoder &d, const T61String &t61) {
  d.expectTag (tT61String);
  uint32 len = d.decodeLength();

  t61.assign (d.getOctets(len), len);

  return d;
}

inline BERDecoder &operator>> (BERDecoder &d, const UTF8String &us) {
  d.expectTag (tUTF8String);
  uint32 len = d.decodeLength();

  us.assign (d.getOctets(len), len);

  return d;
}

inline BERDecoder &operator>> (BERDecoder &d, const VideotexString &vs) {
  d.expectTag (tVideotexString);
  uint32 len = d.decodeLength();

  vs.assign (d.getOctets(len), len);

  return d;
}

inline BERDecoder &operator>> (BERDecoder &d, const ISO646String &is) {
  d.expectTag (tISO646String);
  uint32 len = d.decodeLength();

  is.assign (d.getOctets(len), len);

  return d;
}

// ###TODO: Support instance_of() and enumerated() manipulators

END_ASN1_NS

#include "manip.h"

#endif
