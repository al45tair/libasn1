/* Emacs, this is -*-C++-*- */

#ifndef ASN1_DERENCODER_H_
#define ASN1_DERENCODER_H_

#include "base.h"
#include "Tag.h"
#include "BitString.h"
#include "OID.h"
#include "buffer.h"
#include "strings.h"

#include <vector>
#include <list>
#include <deque>
#include <map>
#include <set>
#include <algorithm>
#include <memory>
#include <functional>

BEGIN_ASN1_NS

class DEREncoder
{
public:
  typedef asn1::buffer<asn1::big_endian> buffer;

private:

  struct State {
    buffer               *s;
    bool                  in_set;

    std::vector<buffer *> set_items;

    State() : s(0), in_set(false) {}
    State(State &&other) : s(other.s), in_set(other.in_set),
                           set_items(std::move(other.set_items))
    {}
    State(const State &other) : s(other.s), in_set(other.in_set),
                                set_items(other.set_items)
    {}

    State &operator=(const State &other) {
      s = other.s;
      in_set = other.in_set;
      set_items = other.set_items;
      return *this;
    }

    State &operator=(State &&other) {
      s = other.s;
      in_set = other.in_set;
      set_items = std::move(other.set_items);
      return *this;
    }
  };

  bool                   _replace_next_tag;
  Tag                    _next_tag;
  PrimitiveOrConstructed _next_tag_constructed;

  buffer             *_s;
  State              *_state;
  std::vector<State>  _stack;
  bool                _release_top;

  allocator          &_alloc;

  static bool ptrcmp(buffer *a, buffer *b) { return *a < *b; }

public:
  DEREncoder(allocator &alloc = dynamic_allocator)
    : _replace_next_tag(false), _stack(1, State()), 
      _release_top(true), _alloc(alloc) {
    _state = &_stack.back();
    _s = _state->s = new buffer(_alloc);
  }
  DEREncoder(buffer &b,
             allocator &alloc = dynamic_allocator)
    : _replace_next_tag(false), _stack(1, State()),
      _release_top(false), _alloc(alloc)
  {
    _state = &_stack.back();
    _s = _state->s = &b;
  }
  ~DEREncoder() {
    std::vector<State>::iterator i;
    for (i = _stack.begin(); i < _stack.end(); ++i) {
      State &st = *i;
      if (_release_top || i != _stack.end() - 1)
        delete st.s;
      for (auto j = st.set_items.begin(); j < st.set_items.end(); ++j)
        delete *j;
    }
  }

  void overrideNextTag(const Tag t, PrimitiveOrConstructed c)
  {
    if (!_replace_next_tag) {
      _replace_next_tag = true;
      _next_tag = t;
      _next_tag_constructed = c;
    }
  }

  void encodeOctet(octet o) { _s->put_octet(o); }
  void encodeOctets(const octet *o, unsigned len) {
    _s->put_octets(o, len);
  }
  void encode(uint16 w) {
    _s->put_uint16(w);
  }
  void encode(uint32 w) {
    _s->put_uint32(w);
  }
  void encode(uint64 w) {
    _s->put_uint64(w);
  }

  unsigned lenTBF(uint32 w) {
    if (w > 0xfffffff)
      return 5;
    if (w > 0x1fffff)
      return 4;
    if (w > 0x3fff)
      return 3;
    if (w > 0x7f)
      return 2;
    return 1;
  }

  void encodeTBF(uint32 w) {
    if (w > 0xfffffff)
      encodeOctet (0x80 | (w >> 28));
    if (w > 0x1fffff)
      encodeOctet (0x80 | (w >> 21));
    if (w > 0x3fff)
      encodeOctet (0x80 | (w >> 14));
    if (w > 0x7f)
      encodeOctet (0x80 | (w >> 7));
    encodeOctet(w & 0x7f);
  }

  void encodeTag(Tag t, PrimitiveOrConstructed c = PRIMITIVE)
  {
    if (_state->in_set) {
      _state->set_items.push_back(new buffer(_alloc));
      _s = _state->set_items.back();
    }

    if (_replace_next_tag) {
      t = _next_tag;
      c = _next_tag_constructed;
      _replace_next_tag = false;
    }

    octet pc = c ? 0x20 : 0;

    if (t.number < 31)
      encodeOctet ((t.tagClass << 6) | pc | t.number);
    else {
      uint32 number = t.number;

      encodeOctet ((t.tagClass << 6) | pc | 0x1f);
      encodeTBF (number);
    }
  }

  void encodeLength(uint32 len)
  {
    if (len <= 0x7f) {
      encodeOctet (len);
    } else {
      uint32 lenbe = machine::to_be (len);

      if (len > 0xffffff) {
        encodeOctet (0x84);
        encodeOctets ((octet *)&lenbe, 4);
      } else if (len > 0xffff) {
        encodeOctet (0x83);
        encodeOctets ((octet *)&lenbe + 1, 3);
      } else if (len > 0xff) {
        encodeOctet (0x82);
        encodeOctets ((octet *)&lenbe + 2, 2);
      } else {
        encodeOctet (0x81);
        encodeOctet (len);
      }
    }

    _s->reserve (len);
  }

  typedef enum {
    SEQUENCE = 0,
    SET = 1
  } PushMode;

  void pushState(PushMode p) {
    _stack.push_back(DEREncoder::State());
    _state = &_stack.back();
    _s = _state->s = new buffer(_alloc);
    _state->in_set = p == SET;
  }
  void popState() {
    State &s = *_state;
    if (s.in_set) {
      std::sort(s.set_items.begin(), s.set_items.end(), ptrcmp);
      for (auto i = s.set_items.begin(); i < s.set_items.end(); ++i) {
        s.s->put_buffer(**i);
        delete *i;
      }
    }
    _state = &_stack[_stack.size() - 2];
    _s = _state->s;
    encodeLength (s.s->length());
    encodeOctets (s.s->data(), s.s->length());
    delete s.s;
    _stack.pop_back();
  }

public:
  const buffer &asDER() const {
    if (_stack.size() != 1)
      throw std::runtime_error("Missing ASN1::end in DER encoding");
    return *_s;
  }
};

inline DEREncoder &operator<< (DEREncoder &e, DEREncoder &(*pf)(DEREncoder &)) {
  return pf(e);
}

inline DEREncoder &operator<< (DEREncoder &e, bool b) {
  e.encodeTag (tBoolean);
  e.encodeLength (1);
  e.encodeOctet (b ? 0xff : 0x00);
  return e;
}

inline DEREncoder &operator<< (DEREncoder &e, int32 i) {
  e.encodeTag (tInteger);

  unsigned lz, extra_zero;

  if (i > 0) {
    lz = machine::clz (i);
    extra_zero = !(lz & 7);
  } else {
    lz = machine::clz (~i) - 1;
    extra_zero = 0;
  }

  uint32 ibe = machine::to_be (i);
  unsigned offset = lz >> 3;
  unsigned bytes = 4 - offset;
 
  e.encodeLength (bytes + extra_zero);
  if (extra_zero)
    e.encodeOctet (0);
  e.encodeOctets ((octet *)&ibe + offset, bytes);

  return e;
}

inline DEREncoder &operator<< (DEREncoder &e, uint32 i) {
  e.encodeTag (tInteger);

  uint32 ibe = machine::to_be (i);
  unsigned lz = machine::clz (i);
  unsigned extra_zero = !(lz & 7);
  unsigned offset = lz >> 3;
  unsigned bytes = 4 - offset;
 
  e.encodeLength (bytes + extra_zero);
  if (extra_zero)
    e.encodeOctet (0);
  e.encodeOctets ((octet *)&ibe + offset, bytes);

  return e;
}

inline DEREncoder &operator<< (DEREncoder &e, int64 i) {
  e.encodeTag (tInteger);

  unsigned lz, extra_zero;

  if (i > 0) {
    lz = machine::clz (i);
    extra_zero = !(lz & 7);
  } else {
    lz = machine::clz (~i) - 1;
    extra_zero = 0;
  }

  uint64 ibe = machine::to_be (i);
  unsigned offset = lz >> 3;
  unsigned bytes = 8 - offset;
 
  e.encodeLength (bytes + extra_zero);
  if (extra_zero)
    e.encodeOctet (0);
  e.encodeOctets ((octet *)&ibe + offset, bytes);

  return e;
}

inline DEREncoder &operator<< (DEREncoder &e, uint64 i) {
  e.encodeTag (tInteger);

  uint64 ibe = machine::to_be (i);
  unsigned lz = machine::clz (i);
  unsigned extra_zero = !(lz & 7);
  unsigned offset = lz >> 3;
  unsigned bytes = 8 - offset;
 
  e.encodeLength (bytes + extra_zero);
  if (extra_zero)
    e.encodeOctet (0);
  e.encodeOctets ((octet *)&ibe + offset, bytes);

  return e;
}

inline DEREncoder &operator<< (DEREncoder &e, double d)
{
  union {
    uint64 u;
    double d;
  } un;
  octet o;

  e.encodeTag (tReal);

  un.d = d;
  uint64 u = un.u;

  /* 8.5.2 If the real value is the value plus zero, there shall be no
     contents octets in the encoding */
  if (u == 0) {
    e.encodeLength (0);
    return e;
  }
  
  /* 8.5.3 If the real value is the value minus zero, then it shall be
     encoded as specified in 8.5.9 */
  if (u == 0x8000000000000000) {
    e.encodeLength (1);
    e.encodeOctet (0x43);
    return e;
  }

  /* 8.5.9 PLUS-INFINITY */
  if (u == 0x7ff0000000000000) {
    e.encodeLength (1);
    e.encodeOctet (0x40);
    return e;
  }
  
  /* 8.5.9 MINUS-INFINITY */
  if (u == 0xfff0000000000000) {
    e.encodeLength (1);
    e.encodeOctet (0x41);
    return e;
  }

  int exponent = (u >> 52) & 0x7ff;
  bool sign = u & 0x8000000000000000;
 
  /* 8.5.9 NOT-A-NUMBER */
  if (exponent == 0x7ff) {
    e.encodeLength (1);
    e.encodeOctet (0x42);
  }

  if (!exponent) {
    // It's subnormal; adjust everything
    unsigned lz;
    u &= 0x000fffffffffffff;
    lz = machine::clz(u) - 11;
    exponent = -1022 - lz;
    u <<= lz;
  } else {
    exponent -= 1023;

    // Add-in the implied 1
    u &= 0x000fffffffffffff;
    u |= 0x0010000000000000;
  }

  // BER has its binary point to the right of the mantissa
  exponent -= 52;

  // Right-justify the mantissa (per X.690 11.3.1)
  unsigned tz = machine::ctz(u);
  u >>= tz;
  exponent += tz;

  // Binary encoding
  o = 0x80;

  // Sign bit
  if (sign)
    o |= 0x40;

  unsigned elen;

  if (exponent >= -128 && exponent < 128) {
    elen = 1;
  } else {
    elen = 2;
    o |= 0x01;
  }

  // Work out how many bytes for the mantissa
  unsigned mofs = machine::clz(u) >> 3;
  unsigned mlen = 8 - mofs;

  // Actually encode stuff
  e.encodeLength (elen + mlen + 1);
  e.encodeOctet (o);
  
  if (elen == 1)
    e.encodeOctet (exponent);
  else
    e.encode ((uint16)exponent);

  u = machine::to_be (u);

  e.encodeOctets (((octet *)&u) + mofs, mlen);

  return e;
}

template <class A>
DEREncoder &operator<< (DEREncoder &e, const std::vector<octet, A> &v) {
  e.encodeTag (tOctetString);
  e.encodeLength (v.size());
  e.encodeOctets (v.data(), v.size());
  return e;
}

template <class A>
DEREncoder &operator<< (DEREncoder &e, const BitString<A> &v) {
  unsigned bytes = (v.size() + 7) >> 3;
  unsigned ignored = 8 - (v.size() & 7);

  e.encodeTag (tBitString);
  e.encodeLength (bytes + 1);
  e.encodeOctet (ignored);
  e.encodeOctets (v.data(), bytes);
  return e;
}

inline DEREncoder &null(DEREncoder &e) {
  e.encodeTag (tNull);
  e.encodeLength (0);
  return e;
}

/* DEREncoder e;

   e << ASN1::sequence << "Smith" << true << ASN1::end;

   e << ASN1::set << "Smith" << true << ASN1::end;

*/
inline DEREncoder &sequence(DEREncoder &e) {
  e.encodeTag (tSequence, CONSTRUCTED);
  e.pushState (DEREncoder::SEQUENCE);
  return e;
}

inline DEREncoder &set(DEREncoder &e) {
  e.encodeTag (tSet, CONSTRUCTED);
  e.pushState (DEREncoder::SET);
  return e;
}

inline DEREncoder &end(DEREncoder &e) {
  e.popState ();
  return e;
}

/* DEREncoder e;
   std::vector<int> v;

   e << v;
  
   is equivalent to

   e << ASN1::sequence << v[0] << v[1] << ... << v[n] << ASN1::end;

   Similarly for std::list and std::deque.
*/
template <class T, class A=std::allocator<T> >
DEREncoder &operator<< (DEREncoder &e, const std::vector<T, A> &v) {
  e.encodeTag (tSequence, CONSTRUCTED);
  e.pushState (DEREncoder::SEQUENCE);
  for (auto i = v.begin(); i < v.end(); ++i)
    e << *i;
  e.popState ();
  return e;
}
template <class T, class A=std::allocator<T> >
DEREncoder &operator<< (DEREncoder &e, const std::list<T, A> &v) {
  e.encodeTag (tSequence, CONSTRUCTED);
  e.pushState (DEREncoder::SEQUENCE);
  for (auto i = v.begin(); i < v.end(); ++i)
    e << *i;
  e.popState ();
  return e;
}
template <class T, class A=std::allocator<T> >
DEREncoder &operator<< (DEREncoder &e, const std::deque<T, A> &v) {
  e.encodeTag (tSequence, CONSTRUCTED);
  e.pushState (DEREncoder::SEQUENCE);
  for (auto i = v.begin(); i < v.end(); ++i)
    e << *i;
  e.popState ();
  return e;
}

/* DEREncoder e;
   std::set<int> s;

   e << s;
  
   is equivalent to

   e << ASN1::set << s[0] << s[1] << ... << s[n] << ASN1::end;

*/
template <class T, class Compare=std::less<T>, class A=std::allocator<T> >
DEREncoder &operator<< (DEREncoder &e, const std::set<T, Compare, A> &v) {
  e.encodeTag (tSet, CONSTRUCTED);
  e.pushState (DEREncoder::SET);
  for (auto i = v.begin(); i < v.end(); ++i)
    e << *i;
  e.popState ();
  return e;
}

/* DEREncoder e;
   std::map<int, int> m;

   e << m;
  
   is equivalent to

   e << ASN1::set
       << ASN1::sequence << key[0] << value[0] << ASN1::end
       << ASN1::sequence << key[1] << value[1] << ASN1::end
       << ...
       << ASN1::sequence << key[n] << value[n] << ASN1::end
     << ASN1::end;

   Similarly for std::multimap.
*/
template <class Key, class T, class Compare=std::less<Key>,
          class A=std::allocator<std::pair<const Key, T> > >
DEREncoder &operator<< (DEREncoder &e, const std::map<Key, T, Compare, A> &m)
{
  e.encodeTag (tSet, CONSTRUCTED);
  e.pushState (DEREncoder::SET);
  for (auto i = m.begin(); i < m.end(); ++i) {
    e.pushState (DEREncoder::SEQUENCE);
    e << i->first << i->second;
    e.popState ();
  }
  e.popState ();
  return e;
}
template <class Key, class T, class Compare=std::less<Key>,
          class A=std::allocator<std::pair<const Key, T> > >
DEREncoder &operator<< (DEREncoder &e, 
                        const std::multimap<Key, T, Compare, A> &m)
{
  e.encodeTag (tSet, CONSTRUCTED);
  e.pushState (DEREncoder::SET);
  for (auto i = m.begin(); i < m.end(); ++i) {
    e.pushState (DEREncoder::SEQUENCE);
    e << i->first << i->second;
    e.popState ();
  }
  e.popState ();
  return e;
}

DEREncoder &operator<< (DEREncoder &e,
                        const OID &o)
{
  e.encodeTag (tOID);

  unsigned subid = o[0] * 40 + o[1];
  unsigned len = e.lenTBF (subid);

  for (auto i = o.begin() + 2; i < o.end(); ++i)
    len += e.lenTBF (*i);

  e.encodeLength (len);

  e.encodeTBF (subid);
  for (auto i = o.begin() + 2; i < o.end(); ++i)
    e.encodeTBF (*i);

  return e;
}

DEREncoder &operator<< (DEREncoder &e,
                        const RelativeOID &o)
{
  e.encodeTag (tRelativeOID);

  unsigned len = 0;
  for (auto i = o.begin(); i < o.end(); ++i)
    len += e.lenTBF (*i);

  e.encodeLength (len);

  for (auto i = o.begin(); i < o.end(); ++i)
    e.encodeTBF (*i);

  return e;
}

// String types
inline DEREncoder &operator<< (DEREncoder &e, const BMPString &bmp) {
  e.encodeTag (tBMPString);
  e.encodeLength(bmp.length() * 2);
  for (auto p = bmp.begin(); p != bmp.end(); ++p)
    e.encode((uint16)*p);
  return e;
}

inline DEREncoder &operator<< (DEREncoder &e, const UniversalString &us) {
  e.encodeTag (tUniversalString);
  e.encodeLength(us.length() * 4);
  for (auto p = us.begin(); p != us.end(); ++p)
    e.encode((uint32)*p);
  return e;
}

inline DEREncoder &operator<< (DEREncoder &e, const GeneralString &gs) {
  e.encodeTag (tGeneralString);
  e.encodeLength (gs.length());
  e.encodeOctets ((const octet *)gs.data(), gs.length());
  return e;
}

inline DEREncoder &operator<< (DEREncoder &e, const GraphicString &gs) {
  e.encodeTag (tGraphicString);
  e.encodeLength (gs.length());
  e.encodeOctets ((const octet *)gs.data(), gs.length());
  return e;
}

inline DEREncoder &operator<< (DEREncoder &e, const IA5String &ia5) {
  e.encodeTag (tIA5String);
  e.encodeLength (ia5.length());
  e.encodeOctets ((const octet *)ia5.data(), ia5.length());
  return e;
}

inline DEREncoder &operator<< (DEREncoder &e, const NumericString &ns) {
  e.encodeTag (tNumericString);
  e.encodeLength (ns.length());
  e.encodeOctets ((const octet *)ns.data(), ns.length());
  return e;
}

inline DEREncoder &operator<< (DEREncoder &e, const PrintableString &ps) {
  e.encodeTag (tPrintableString);
  e.encodeLength (ps.length());
  e.encodeOctets ((const octet *)ps.data(), ps.length());
  return e;
}

inline DEREncoder &operator<< (DEREncoder &e, const T61String &t61s) {
  e.encodeTag (tT61String);
  e.encodeLength (t61s.length());
  e.encodeOctets ((const octet *)t61s.data(), t61s.length());
  return e;
}

inline DEREncoder &operator<< (DEREncoder &e, const UTF8String &us) {
  e.encodeTag (tUTF8String);
  e.encodeLength (us.length());
  e.encodeOctets ((const octet *)us.data(), us.length());
  return e;
}

inline DEREncoder &operator<< (DEREncoder &e, const VideotexString &vs) {
  e.encodeTag (tVideotexString);
  e.encodeLength (vs.length());
  e.encodeOctets ((const octet *)vs.data(), vs.length());
  return e;
}

inline DEREncoder &operator<< (DEREncoder &e, const ISO646String &is) {
  e.encodeTag (tISO646String);
  e.encodeLength (is.length());
  e.encodeOctets ((const octet *)is.data(), is.length());
  return e;
}

// ###TODO: Encode lots more things

END_ASN1_NS

#include "manip.h"

#endif /* ASN1_DERENCODER_H_ */
