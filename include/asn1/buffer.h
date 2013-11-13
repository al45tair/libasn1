/* Emacs, this is -*-C++-*- */

#ifndef ASN1_BUFFER_H_
#define ASN1_BUFFER_H_

#include "base.h"
#include "machine.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iomanip>

BEGIN_ASN1_NS

class native_endian
{
public:
  static void write_u16 (octet *p, uint16 u)   { *(uint16 *)p = u; }
  static void write_u32 (octet *p, uint32 u)   { *(uint32 *)p = u; }
  static void write_u64 (octet *p, uint64 u)   { *(uint64 *)p = u; }
  static void write_i16 (octet *p, int16 u)    { *(int16 *)p = u; }
  static void write_i32 (octet *p, int32 u)    { *(int32 *)p = u; }
  static void write_i64 (octet *p, int64 u)    { *(int64 *)p = u; }
  static void write_f (octet *p, float f)      { *(float *)p = f; }
  static void write_d (octet *p, double d)     { *(double *)p = d; }

  static uint16 read_u16 (octet *p)  { return *(uint16 *)p; }
  static uint32 read_u32 (octet *p)  { return *(uint32 *)p; }
  static uint64 read_u64 (octet *p)  { return *(uint64 *)p; }
  static int16 read_i16 (octet *p)   { return *(int16 *)p; }
  static int32 read_i32 (octet *p)   { return *(int32 *)p; }
  static int64 read_i64 (octet *p)   { return *(int64 *)p; }
  static float read_f (octet *p)     { return *(float *)p; }
  static double read_d (octet *p)    { return *(double *)p; }
};

class big_endian
{
public:
  static void write_u16 (octet *p, uint16 u)   { *(uint16 *)p = machine::to_be (u); }
  static void write_u32 (octet *p, uint32 u)   { *(uint32 *)p = machine::to_be (u); }
  static void write_u64 (octet *p, uint64 u)   { *(uint64 *)p = machine::to_be (u); }
  static void write_i16 (octet *p, int16 u)    { *(int16 *)p = machine::to_be (u); }
  static void write_i32 (octet *p, int32 u)    { *(int32 *)p = machine::to_be (u); }
  static void write_i64 (octet *p, int64 u)    { *(int64 *)p = machine::to_be (u); }
  static void write_f (octet *p, float f)      { *(uint32 *)p = machine::to_bef (f); }
  static void write_d (octet *p, double d)     { *(uint64 *)p = machine::to_bef (d); }

  static uint16 read_u16 (octet *p)  { return machine::from_be (*(uint16 *)p); }
  static uint32 read_u32 (octet *p)  { return machine::from_be (*(uint32 *)p); }
  static uint64 read_u64 (octet *p)  { return machine::from_be (*(uint64 *)p); }
  static int16 read_i16 (octet *p)   { return machine::from_be (*(int16 *)p); }
  static int32 read_i32 (octet *p)   { return machine::from_be (*(int32 *)p); }
  static int64 read_i64 (octet *p)   { return machine::from_be (*(int64 *)p); }
  static float read_f (octet *p)     { return machine::from_bef (*(uint32 *)p); }
  static double read_d (octet *p)    { return machine::from_bef (*(uint64 *)p); }
};

class little_endian
{
public:
  static void write_u16 (octet *p, uint16 u)   { *(uint16 *)p = machine::to_be (u); }
  static void write_u32 (octet *p, uint32 u)   { *(uint32 *)p = machine::to_be (u); }
  static void write_u64 (octet *p, uint64 u)   { *(uint64 *)p = machine::to_be (u); }
  static void write_i16 (octet *p, int16 u)    { *(int16 *)p = machine::to_be (u); }
  static void write_i32 (octet *p, int32 u)    { *(int32 *)p = machine::to_be (u); }
  static void write_i64 (octet *p, int64 u)    { *(int64 *)p = machine::to_be (u); }
  static void write_f (octet *p, float f)      { *(uint32 *)p = machine::to_bef (f); }
  static void write_d (octet *p, double d)     { *(uint64 *)p = machine::to_bef (d); }

  static uint16 read_u16 (octet *p)  { return machine::from_be (*(uint16 *)p); }
  static uint32 read_u32 (octet *p)  { return machine::from_be (*(uint32 *)p); }
  static uint64 read_u64 (octet *p)  { return machine::from_be (*(uint64 *)p); }
  static int16 read_i16 (octet *p)   { return machine::from_be (*(int16 *)p); }
  static int32 read_i32 (octet *p)   { return machine::from_be (*(int32 *)p); }
  static int64 read_i64 (octet *p)   { return machine::from_be (*(int64 *)p); }
  static float read_f (octet *p)     { return machine::from_bef (*(uint32 *)p); }
  static double read_d (octet *p)    { return machine::from_bef (*(uint64 *)p); }
};

class allocator {
public:
  virtual octet *resize (octet *b, size_t len, size_t new_capacity) = 0;
  virtual void release (octet *b, size_t len) = 0;
};

class fixed_alloc_class : public allocator
{
public:
  octet *resize (octet *b, size_t len, size_t new_capacity) {
    (void)b, (void)len, (void)new_capacity;
    throw std::runtime_error("Cannot resize fixed-size buffer");
  }
  void release (octet *b, size_t len) {
    (void)b, (void)len;
  }
};

extern fixed_alloc_class fixed_allocator;

class dynamic_alloc_class : public allocator
{
public:
  octet *resize (octet *b, size_t len, size_t new_capacity) {
    octet *nb = (octet *)std::realloc (b, new_capacity);
    (void)len;
    if (!nb)
      throw std::runtime_error("Out of memory");
    return nb;
  }
  void release (octet *b, size_t len) {
    (void)len;
    if (b) {
      std::free (b);
    }
  }
};

extern dynamic_alloc_class dynamic_allocator;

class secure_alloc_class : public allocator
{
public:
  octet *resize (octet *b, size_t len, size_t new_capacity) {
    octet *nb = (octet *)std::malloc (new_capacity);
    if (!nb)
      throw std::runtime_error("Out of memory");
    if (b) {
      std::memcpy (nb, b, len);
      std::memset (b, 0, len);
      std::free (b);
    }
    return nb;
  }
  void release (octet *b, size_t len) {
    if (b) {
      std::memset (b, 0, len);
      std::free (b);
    }
  }
};

extern secure_alloc_class secure_allocator;

template <class Endian=native_endian>
class buffer
{
public:
  class reader;

protected:
  typedef Endian endianness;

  allocator &a;
  octet *b, *p, *e;

  void resize (size_t cap) {
    octet *nb = a.resize (b, e - b, cap);
    p += nb - b;
    b = nb;
    e = nb + cap;
  }

  void grow (size_t n = 64) {
    resize (capacity() + ((n + 63) & ~63));
  }
  
  buffer(octet *base, octet *ptr, octet *end) : b(base), p(ptr), e(end) {}

public:
  buffer(allocator &alloc = dynamic_allocator) : a(alloc), b(0), p(0), e(0) {}
  buffer (octet *ptr, size_t len, allocator &alloc = fixed_allocator) : 
    a(alloc), b(ptr), p(ptr), e(ptr + len) {}
  ~buffer() { a.release(b, e - b); }

  const octet *data() const { return b; }
  size_t capacity() const { return e - b; }
  size_t length() const { return p - b; }
  void reserve(size_t space) {
    if (static_cast<size_t>(e - p) < space)
      grow (space);
  }
  reader begin() const;

  void put_octet (octet o) {
    if (p >= e) grow();
    *p++ = o;
  }
  void put_uint16 (uint16 u) {
    if (e - p < 2) grow();
    endianness::write_u16 (p, u);
    p += 2;
  }
  void put_uint32 (uint32 u) {
    if (e - p < 4) grow();
    endianness::write_u32 (p, u);
    p += 4;
  }
  void put_uint64 (uint64 u) {
    if (e - p < 8) grow();
    endianness::write_u64 (p, u);
    p += 8;
  }
  void put_int16 (int16 u) {
    if (e - p < 2) grow();
    endianness::write_i16 (p, u);
    p += 2;
  }
  void put_int32 (int32 u) {
    if (e - p < 4) grow();
    endianness::write_i32 (p, u);
    p += 4;
  }
  void put_int64 (int64 u) {
    if (e - p < 8) grow();
    endianness::write_i64 (p, u);
    p += 8;
  }
  void put_float (float f) {
    if (e - p < 4) grow();
    endianness::write_f (p, f);
    p += 4;
  }
  void put_double (double d) {
    if (e - p < 8) grow();
    endianness::write_d (p, d);
    p += 8;
  }
  void put_octets (const octet *o, size_t len) {
    if (static_cast<size_t>(e - p) < len) grow (len);
    std::memcpy (p, o, len);
    p += len;
  }
  void put_buffer (const buffer &other) {
    put_octets (other.data(), other.length());
  }

  buffer(const buffer &other) : b(0), p(0), e(0) {
    put_buffer (other);
  }

  buffer(buffer &&other) : b(other.b), p(other.p), e(other.e) {
    other.b = other.p = other.e = 0;
  }

  buffer &operator=(const buffer &other) {
    size_t len = other.length();
    resize(other.length());
    std::memcpy (p, other.p, len);
    return *this;
  }
  buffer &operator=(buffer &&other) {
    b = other.b;
    p = other.p;
    e = other.e;
    other.b = other.p = other.e = 0;
    return *this;
  }
};

template <class Endian>
inline bool operator==(const buffer<Endian> &a, 
                       const buffer<Endian> &b)
{
  return (a.length() == b.length() 
          && std::memcmp (a.data(), b.data(), a.length()) == 0);
}
template <class Endian>
inline bool operator!=(const buffer<Endian> &a,
                       const buffer<Endian> &b)
{
  return (a.length() != b.length() 
          || std::memcmp (a.data(), b.data(), a.length()) != 0);
}
template <class Endian>
inline bool operator<(const buffer<Endian> &a, 
                      const buffer<Endian> &b)
{
  size_t mlen = std::min(a.length(), b.length());
  if (std::memcmp (a.data(), b.data(), mlen) < 0)
    return true;
  return a.length() < b.length();
}
template <class Endian>
inline bool operator<=(const buffer<Endian> &a,
                       const buffer<Endian> &b)
{
  size_t mlen = std::min(a.length(), b.length());
  if (std::memcmp (a.data(), b.data(), mlen) <= 0)
    return true;
  return a.length() <= b.length();
}
template <class Endian>
inline bool operator>(const buffer<Endian> &a, 
                      const buffer<Endian> &b)
{
  size_t mlen = std::min(a.length(), b.length());
  if (std::memcmp (a.data(), b.data(), mlen) > 0)
    return true;
  return a.length() > b.length();
}
template <class Endian>
inline bool operator>=(const buffer<Endian> &a,
                       const buffer<Endian> &b)
{
  size_t mlen = std::min(a.length(), b.length());
  if (std::memcmp (a.data(), b.data(), mlen) >= 0)
    return true;
  return a.length() >= b.length();
}

template <class Endian>
std::ostream &operator<< (std::ostream &os, const buffer<Endian> &b)
{
  std::ios_base::fmtflags oldFlags = os.flags();
  std::streamsize oldPrec = os.precision();
  char oldFill = os.fill();

  os << std::hex << std::noshowbase << std::internal << std::setfill('0');

  const octet *base = b.data();
  const octet *end = b.data() + b.length();
  for (const octet *p = base; p < end; ++p) {
    unsigned count = p - base;
    if (count & 0xf)
      os << " ";
    else {
      if (count)
        os << std::endl;
      os << std::setw(8) << count << "  ";
    }
    ++count;

    os << std::setw(2) << unsigned(*p);
  }

  os << std::endl;

  os.flags(oldFlags);
  os.precision(oldPrec);
  os.fill(oldFill);

  return os;  
}

template <class Endian>
class buffer<Endian>::reader
{
private:
  typedef Endian endianness;

  buffer &b;
  octet *p;

  void oob() { throw std::runtime_error("Attempt to read past end of buffer"); }

  reader();
public:
  reader(const reader &other) : b(other.b), p(other.p) {}
  reader(const buffer &buffer) : b(buffer), p(b.b) {}

  octet get_octet () { if (p >= b.p) oob(); return *p++; }
  uint16 get_uint16 () {
    if (b.p - p < 2) oob();
    uint16 r = endianness::read_u16 (p);
    p += 2;
    return r;
  }
  uint32 get_uint32 () {
    if (b.p - p < 4) oob();
    uint32 r = endianness::read_u32 (p);
    p += 4;
    return r;
  }
  uint64 get_uint64 () {
    if (b.p - p < 8) oob();
    uint64 r = endianness::read_u64 (p);
    p += 8;
    return r;
  }
  int16 get_int16 () {
    if (b.p - p < 2) oob();
    int16 r = endianness::read_i16 (p);
    p += 2;
    return r;
  }
  int32 get_int32 () {
    if (b.p - p < 4) oob();
    uint32 r = endianness::read_i32 (p);
    p += 4;
    return r;
  }
  int64 get_int64 () {
    if (b.p - p < 8) oob();
    uint64 r = endianness::read_i64 (p);
    p += 8;
    return r;
  }
  float get_float () {
    if (b.p - p < 4) oob();
    float r = endianness::read_f (p);
    p += 4;
    return r;
  }
  double get_double () {
    if (b.p - p < 8) oob();
    double r = endianness::read_d (p);
    p += 8;
    return r;
  }
};

template <class Endian>
typename buffer<Endian>::reader buffer<Endian>::begin() const {
  return reader(*this);
}

END_ASN1_NS

#endif
