/* Emacs, this is -*-C++-*- */

#ifndef ASN1_MACHINE_H_
#define ASN1_MACHINE_H_

namespace asn1 {

namespace machine {

template <unsigned ss = sizeof(short), unsigned is = sizeof(int),
          unsigned ls = sizeof(long), unsigned lls = sizeof(long long),
          unsigned sp = sizeof(void *)>
struct tsel_ {
};

// ILP32
template <>
struct tsel_<2, 4, 4, 8, 4> {
  typedef short              int16;
  typedef unsigned short     uint16;
  typedef int                int32;
  typedef unsigned int       uint32;
  typedef long long          int64;
  typedef unsigned long long uint64;
  typedef int32              intptr;
  typedef uint32             uintptr;
  typedef int32              word;
  typedef uint32             uword;
};

// LP64
template <>
struct tsel_<2, 4, 8, 8, 8> {
  typedef short              int16;
  typedef unsigned short     uint16;
  typedef int                int32;
  typedef unsigned int       uint32;
  typedef long               int64;
  typedef unsigned long      uint64;
  typedef int64              intptr;
  typedef uint64             uintptr;
  typedef int64              word;
  typedef uint64             uword;
};

// P64
template <>
struct tsel_<2, 4, 4, 8, 8> {
  typedef short              int16;
  typedef unsigned short     uint16;
  typedef int                int32;
  typedef unsigned int       uint32;
  typedef long long          int64;
  typedef unsigned long long uint64;
  typedef int64              intptr;
  typedef uint64             uintptr;
  typedef int64              word;
  typedef uint64             uword;
};

typedef tsel_<> types;

typedef char               int8;
typedef unsigned char      uint8;
typedef types::int16       int16;
typedef types::uint16      uint16;
typedef types::int32       int32;
typedef types::uint32      uint32;
typedef types::int64       int64;
typedef types::uint64      uint64;
typedef types::word        word;
typedef types::uword       uword;

const unsigned word_size = sizeof(word);
const unsigned word_bits = word_size * 8;
const uword top_bit = static_cast<uword>(1) << (word_bits - 1);
const uword all_ones = ~static_cast<uword>(0);

inline uint32 pop (uint32 i) {
  i = ((i >> 1) & 0x55555555) + (i & 0x55555555);
  i = ((i >> 2) & 0x33333333) + (i & 0x33333333);
  i = ((i >> 4) + i)  & 0x0f0f0f0f;
  i = ((i >> 8) + i)  & 0x00ff00ff;
  i = ((i >> 16) + i) & 0x0000ffff;
  return i;
}

inline uint64 pop (uint64 i) {
  return (pop(static_cast<uint32>(i >> 32))
          + pop(static_cast<uint32>(i)));
}

inline bool is_big_endian() {
  union { uint32 i; uint8 c[4]; } bint = { 0x01020304 };
  return bint.c[0] == 1;
}

// These are intended to be obvious to the compiler
inline uint16 bswap (uint16 i) {
  return ((i & 0xff00) >> 8) | ((i & 0xff) << 8); 
}
inline uint32 bswap (uint32 i) {
  return (  ((i & 0xff000000) >> 24)
          | ((i & 0x00ff0000) >> 8)
          | ((i & 0x0000ff00) << 8)
          | ((i & 0x000000ff) << 24));
}
inline uint64 bswap (uint64 i) {
  return (  ((i & 0xff00000000000000) >> 56)
          | ((i & 0x00ff000000000000) >> 40)
          | ((i & 0x0000ff0000000000) >> 24)
          | ((i & 0x000000ff00000000) >> 8)
          | ((i & 0x00000000ff000000) << 8)
          | ((i & 0x0000000000ff0000) << 24)
          | ((i & 0x000000000000ff00) << 40)
          | ((i & 0x00000000000000ff) << 56));
}

inline uint32 clz (uint32 i) {
  i |= i >> 1; i |= i >> 2;
  i |= i >> 4; i |= i >> 8;
  i |= i >> 16;

  i = ~i;

  return pop(i);
}
inline uint32 clz (int32 i) { return clz(static_cast<uint32>(i)); }

inline uint64 clz (uint64 i) {
  i |= i >> 1; i |= i >> 2;
  i |= i >> 4; i |= i >> 8;
  i |= i >> 16; i |= i >> 32;

  i = ~i;

  return pop(i);
}
inline uint64 clz (int64 i) { return clz(static_cast<uint64>(i)); }

inline uint32 ctz (uint32 i) {
  i |= i << 1; i |= i << 2;
  i |= i << 4; i |= i << 8;
  i |= i << 16;

  i = ~i;

  return pop(i);
}
inline uint32 ctz (int32 i) { return ctz(static_cast<uint32>(i)); }

inline uint64 ctz (uint64 i) {
  i |= i << 1; i |= i << 2;
  i |= i << 4; i |= i << 8;
  i |= i << 16; i |= i << 32;

  i = ~i;

  return pop(i);
}
inline uint64 ctz (int64 i) { return clz(static_cast<uint64>(i)); }

inline uint16 to_be(uint16 i) {
  if (is_big_endian()) return i; else return bswap(i);
}
inline uint16 to_be(int16 i) { return to_be(static_cast<uint16>(i)); }
inline uint32 to_be(uint32 i) { 
  if (is_big_endian()) return i; else return bswap(i);
}
inline uint32 to_be(int32 i) { return to_be(static_cast<uint32>(i)); }
inline uint64 to_be(uint64 i) { 
  if (is_big_endian()) return i; else return bswap(i);
}
inline uint64 to_be(int64 i) { return to_be(static_cast<uint64>(i)); }

inline uint16 from_be(uint16 i) {
  if (is_big_endian()) return i; else return bswap(i);
}
inline uint16 from_be(int16 i) { return to_be(static_cast<uint16>(i)); }
inline uint32 from_be(uint32 i) { 
  if (is_big_endian()) return i; else return bswap(i);
}
inline uint32 from_be(int32 i) { return to_be(static_cast<uint32>(i)); }
inline uint64 from_be(uint64 i) { 
  if (is_big_endian()) return i; else return bswap(i);
}
inline uint64 from_be(int64 i) { return to_be(static_cast<uint64>(i)); }

inline uint32 to_bef(float f) {
  union { float f; uint32 u; } u = { .f = f };
  return to_be(u.u);
}
inline float from_bef(uint32 i) {
  union { float f; uint32 u; } u = { .u = from_be(i) };
  return u.f;
}
inline uint64 to_bef(double d) {
  union { double d; uint64 u; } u = { .d = d };
  return to_be(u.u);
}
inline double from_bef(uint64 i) {
  union { double d; uint64 u; } u = { .u = from_be(i) };
  return u.d;
}

inline uint16 to_le(uint16 i) {
  if (!is_big_endian()) return i; else return bswap(i);
}
inline uint16 to_le(int16 i) { return to_le(static_cast<uint16>(i)); }
inline uint32 to_le(uint32 i) { 
  if (!is_big_endian()) return i; else return bswap(i);
}
inline uint32 to_le(int32 i) { return to_le(static_cast<uint32>(i)); }
inline uint64 to_le(uint64 i) { 
  if (!is_big_endian()) return i; else return bswap(i);
}
inline uint64 to_le(int64 i) { return to_le(static_cast<uint64>(i)); }

inline uint16 from_le(uint16 i) {
  if (!is_big_endian()) return i; else return bswap(i);
}
inline uint16 from_le(int16 i) { return to_le(static_cast<uint16>(i)); }
inline uint32 from_le(uint32 i) { 
  if (!is_big_endian()) return i; else return bswap(i);
}
inline uint32 from_le(int32 i) { return to_le(static_cast<uint32>(i)); }
inline uint64 from_le(uint64 i) { 
  if (!is_big_endian()) return i; else return bswap(i);
}
inline uint64 from_le(int64 i) { return to_le(static_cast<uint64>(i)); }

inline uint32 to_lef(float f) {
  union { float f; uint32 u; } u = { .f = f };
  return to_le(u.u);
}
inline float from_lef(uint32 i) {
  union { float f; uint32 u; } u = { .u = from_le(i) };
  return u.f;
}
inline uint64 to_lef(double d) {
  union { double d; uint64 u; } u = { .d = d };
  return to_le(u.u);
}
inline double from_lef(uint64 i) {
  union { double d; uint64 u; } u = { .u = from_le(i) };
  return u.d;
}

}

}

#endif /* ASN1_MACHINE_H_ */
