/* Emacs, this is -*-C++-*- */

#ifndef ASN1_BITSTRING_H_
#define ASN1_BITSTRING_H_

#include "base.h"

#include <cstddef>
#include <cstring>
#include <iterator>
#include <limits>
#include <string>
#include <ostream>
#include <memory>
#include <vector>

BEGIN_ASN1_NS

/* This is a bit like std::vector<bool>, except that we know the internal
   memory layout and it has a few additional constructors.  It shares that
   type's bit-addressable nature.

   Memory-wise, the bits are organised into a byte array, with bit zero
   the top bit of the very first byte.  Bits after the end of the array
   are set to zero. */
template <class Alloc=std::allocator<machine::uword> >
class BitString
{
private:
  typedef std::vector<machine::uword, Alloc> storage;

public:
  typedef bool value_type;
  class reference;
  typedef bool const_reference;
  class pointer;
  class const_pointer;
  typedef Alloc allocator_type;
  typedef pointer iterator;
  typedef const_pointer const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  typedef std::ptrdiff_t difference_type;
  typedef std::size_t size_type;

private:
  storage   _storage;
  size_type _len;

  void _fix_back() {
    unsigned bits = _len % machine::word_bits;
    if (bits) {
      machine::uword mask = machine::to_be (~(machine::all_ones >> bits));
      _storage.back() &= mask;
    }
  }

public:
  BitString(const BitString &other) 
    : _storage(other._storage), _len(other._len) { }
  BitString(const BitString &other, const allocator_type &alloc) 
    : _storage(other._storage, alloc), _len(other._len) { }
  BitString(size_type bits, value_type v)
    : _storage((bits + machine::word_bits - 1) / machine::word_bits,
               v ? machine::all_ones : 0),
      _len(bits) {
    _fix_back ();
  }
  BitString(size_type bits, value_type v,
            const allocator_type &alloc=allocator_type())
    : _storage((bits + machine::word_bits - 1) / machine::word_bits,
               v ? machine::all_ones : 0,
               alloc),
      _len(bits) {
    _fix_back ();
  }
  BitString(const octet *ptr, size_type bits,
            const allocator_type &alloc=allocator_type()) 
    : _storage(alloc) {
    assign (ptr, bits);
  }

  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;
  reverse_iterator rbegin();
  const_reverse_iterator rbegin() const;
  reverse_iterator rend();
  const_reverse_iterator rend() const;
  
  size_type size() const { return _len; }
  size_type max_size() const { 
    return _storage.max_size() * machine::word_bits;
  }
  void resize (size_type n, value_type val = value_type()) {
    machine::word v = val ? machine::all_ones : 0;
    _storage.resize ((n + machine::word_bits - 1) / machine::word_bits, v);
    _len = n;
    _fix_back ();
  }
  size_type capacity() const {
    return _storage.capacity() * machine::word_bits;
  }
  bool empty() const { return !_len; }
  void reserve (size_type n) {
    _storage.reserve ((n + machine::word_bits - 1) / machine::word_bits);
  }

  reference operator[](difference_type d);
  const_reference operator[](difference_type d) const;

  reference at(difference_type d);
  const_reference at(difference_type d) const;

  reference front();
  const_reference front() const;

  reference back();
  const_reference back() const;

  template <class it>
  void assign(it b, it e) {
    assign (b, e, std::iterator_traits<it>::iterator_category());
  }
  template <class it>
  void assign(it b, it e, std::forward_iterator_tag) {
    _storage.reserve (((e - b) + machine::word_bits - 1) / machine::word_bits);
    machine::uword w = 0;
    _len = 0;
    for (it i = b; i < e; ++i) {
      w |= *i ? 1 : 0;
      if (!(++_len % machine::word_bits))
        _storage.push_back (machine::to_be (w));
      w <<= 1;
    }
    if (_len % machine::word_bits)
      _storage.push_back (machine::to_be (w));
  }
  template <class it>
  void assign(it b, it e, std::input_iterator_tag) {
    _storage.clear();
    machine::uword w = 0;
    _len = 0;
    for (it i = b; i < e; ++i) {
      w |= *i ? 1 : 0;
      if (!(++_len % machine::word_bits))
        _storage.push_back (machine::to_be (w));
      w <<= 1;
    }
    if (_len % machine::word_bits)
      _storage.push_back (machine::to_be (w));
  }

  template <class it>
  BitString(it first, it last, const allocator_type &alloc=allocator_type())
    : _storage(alloc)
  {
    assign (first, last);
  }

  void assign(size_type n, const value_type val) {
    machine::word v = val ? machine::all_ones : 0;
    _storage.assign ((n + machine::word_bits - 1) / machine::word_bits, v);
    _len = n;
  }

  void assign(const octet *ptr, size_type bits) {
    size_type full_words = bits / machine::word_bits;
    size_type trailing_bytes = (((bits + 7) / 8)
                                - full_words * sizeof (machine::word));
    const machine::uword *pw = (const machine::uword *)ptr;
    _storage.clear();
    _storage.reserve ((bits + machine::word_bits - 1) / machine::word_bits);
    _storage.assign (pw, pw + full_words);
    if (trailing_bytes) {
      machine::uword w = 0;

      ptr += sizeof (machine::uword) * full_words;
      const octet *pend = ptr + trailing_bytes;
      while (ptr < pend)
        w = (w << 8) | *ptr++;
      w <<= machine::word_bits - 8 * trailing_bytes;
      _storage.push_back (machine::to_be (w));
      _fix_back ();
    }
    _len = bits;
  }

  void push_back(bool b) {
    unsigned bit = _len % machine::word_bits;
    machine::uword mask
      = machine::to_be (static_cast<machine::uword>(1)
                        << (machine::word_bits - bit));

    if (bit) {
      if (b)
        _storage.back() |= mask;
      else
        _storage.back() &= ~mask;
    } else {
      _storage.push_back (mask);
    }

    ++_len;
  }

  void pop_back() {
    if (_len) {
      --_len;

      if (!(_len % machine::word_bits))
        _storage.pop_back ();
    }
  }

  const octet *data() const { return (const octet *)_storage.data(); }

  iterator insert(iterator position, value_type val);
  iterator insert (iterator position, size_type n, value_type val);
  template <class it>
  iterator insert (iterator position, it b, it e);

  // forward iterators can reserve space in advance
  template <class it>
  iterator insert (iterator position, it b, it e, std::forward_iterator_tag);

  // otherwise, use a temporary BitString
  template <class it>
  iterator insert (iterator position, it b, it e, std::input_iterator_tag);

  iterator erase(iterator position);
  iterator erase(iterator b, iterator e);

  void swap(BitString &other) {
    std::swap (_storage, other._storage);
    std::swap (_len, other._len);
  }

  void clear() {
    _storage.clear();
    _len = 0;
  }

  friend bool operator==(const BitString &a, const BitString &b);
  friend bool operator!=(const BitString &a, const BitString &b);
  friend bool operator<(const BitString &a, const BitString &b);
  friend bool operator<=(const BitString &a, const BitString &b);
  friend bool operator>(const BitString &a, const BitString &b);
  friend bool operator>=(const BitString &a, const BitString &b);

  friend BitString operator~(const BitString &a);
  friend BitString operator&(const BitString &a, const BitString &b);
  friend BitString operator|(const BitString &a, const BitString &b);
  friend BitString operator^(const BitString &a, const BitString &b);
};

// We use std::memcmp for some of these because we *want* byte-wise compare
template <class Alloc>
bool operator==(const BitString<Alloc> &a, const BitString<Alloc> &b) {
  return (a._len == b._len 
          && std::memcmp (a._storage.data(), b._storage.data(), a._len / 8) == 0);
}
template <class Alloc>
bool operator!=(const BitString<Alloc> &a, const BitString<Alloc> &b) {
  return (a._len != b._len 
          || std::memcmp (a._storage.data(), b._storage.data(), a._len / 8) != 0);
}
template <class Alloc>
bool operator<(const BitString<Alloc> &a, const BitString<Alloc> &b) {
  return (a._len < b._len 
          || std::memcmp (a._storage.data(), b._storage.data(), a._len / 8) < 0);
}
template <class Alloc>
bool operator<=(const BitString<Alloc> &a, const BitString<Alloc> &b) {
  return (a._len <= b._len 
          || std::memcmp (a._storage.data(), b._storage.data(), a._len / 8) <= 0);
}
template <class Alloc>
bool operator>(const BitString<Alloc> &a, const BitString<Alloc> &b) {
  return (a._len > b._len
          || std::memcmp (a._storage.data(), b._storage.data(), b._len / 8) > 0);
}
template <class Alloc>
bool operator>=(const BitString<Alloc> &a, const BitString<Alloc> &b) {
  return (a._len >= b._len 
          || std::memcmp (a._storage.data(), b._storage.data(), b._len / 8) >= 0);
}

template <class Alloc>
BitString<Alloc> operator~(const BitString<Alloc> &a);
template <class Alloc>
BitString<Alloc> operator&(const BitString<Alloc> &a, const BitString<Alloc> &b);
template <class Alloc>
BitString<Alloc> operator|(const BitString<Alloc> &a, const BitString<Alloc> &b);
template <class Alloc>
BitString<Alloc> operator^(const BitString<Alloc> &a, const BitString<Alloc> &b);

template <class Alloc>
class BitString<Alloc>::pointer {
private:
  machine::uword *_p;
  machine::uword _m;

  friend class BitString;
  friend class BitString::reference;
  pointer();
  pointer(machine::uword *p, machine::uword m) : _p(p), _m(m) {}

public:
  BitString<Alloc>::reference operator[](BitString::difference_type d) const;
  BitString<Alloc>::reference operator*() const;

  friend BitString::pointer operator+(const BitString::pointer &a,
                                      BitString::difference_type b);
  friend BitString::pointer operator-(const BitString::pointer &a,
                                      BitString::difference_type b);
  friend BitString::pointer operator+(BitString::difference_type a,
                                      const BitString::pointer &b);
  friend BitString::difference_type operator-(const BitString::pointer &a,
                                              const BitString::pointer &b);
};

template <class Alloc>
inline typename BitString<Alloc>::pointer 
operator+(const typename BitString<Alloc>::pointer &a,
          typename BitString<Alloc>::difference_type b) {
  unsigned words = b / machine::word_bits;
  unsigned bits = b % machine::word_bits;
  machine::uword mask = (a._m >> bits) | (a._m << (machine::word_bits - bits));
  return BitString<Alloc>::pointer (a._p + words, mask);
}

template <class Alloc>
inline typename BitString<Alloc>::pointer
operator-(const typename BitString<Alloc>::pointer &a,
          typename BitString<Alloc>::difference_type b) {
  unsigned words = b / machine::word_bits;
  unsigned bits = b % machine::word_bits;
  machine::uword mask = (a._m << bits) | (a._m >> (machine::word_bits - bits));
  return BitString<Alloc>::pointer (a._p - words, mask);
}

template <class Alloc>
inline typename BitString<Alloc>::pointer 
operator+(typename BitString<Alloc>::difference_type a,
          const typename BitString<Alloc>::pointer &b) {
  return b + a;
}

template <class Alloc>
inline typename BitString<Alloc>::difference_type 
operator-(const typename BitString<Alloc>::pointer &a,
          const typename BitString<Alloc>::pointer &b) {
  return ((a._p - b._p) * machine::word_bits
          + machine::clz(a._m) - machine::clz(b._m));
}

template <class Alloc>
class BitString<Alloc>::const_pointer {
private:
  const machine::uword *_p;
  machine::uword _m;

  friend class BitString;
  const_pointer();
  const_pointer(const machine::uword *p, machine::uword m) : _p(p), _m(m) {}

public:
  const_pointer(const BitString::pointer &p) : _p(p._p), _m(p._m) {}

  bool operator[](BitString::difference_type d) const;
  bool operator*() const {
    return machine::from_be(*_p) & _m;
  }

  friend BitString::const_pointer operator+(const BitString::const_pointer &a,
                                            BitString::difference_type b);
  friend BitString::const_pointer operator-(const BitString::const_pointer &a,
                                            BitString::difference_type b);
  friend BitString::const_pointer operator+(BitString::difference_type a,
                                            const BitString::const_pointer &b);
  friend BitString::difference_type operator-(const BitString::const_pointer &a,
                                              const BitString::const_pointer &b);
};

template <class Alloc>
inline typename BitString<Alloc>::const_pointer 
operator+(const typename BitString<Alloc>::const_pointer &a,
          typename BitString<Alloc>::difference_type b) {
  unsigned words = b / machine::word_bits;
  unsigned bits = b % machine::word_bits;
  machine::uword mask = (a._m >> bits) | (a._m << (machine::word_bits - bits));
  return BitString<Alloc>::const_pointer (a._p + words, mask);
}

template <class Alloc>
inline typename BitString<Alloc>::const_pointer
operator-(const typename BitString<Alloc>::const_pointer &a,
          typename BitString<Alloc>::difference_type b) {
  unsigned words = b / machine::word_bits;
  unsigned bits = b % machine::word_bits;
  machine::uword mask = (a._m << bits) | (a._m >> (machine::word_bits - bits));
  return BitString<Alloc>::const_pointer (a._p - words, mask);
}

template <class Alloc>
inline typename BitString<Alloc>::const_pointer
operator+(typename BitString<Alloc>::difference_type a,
          const typename BitString<Alloc>::const_pointer &b) {
  return b + a;
}

template <class Alloc>
inline typename BitString<Alloc>::difference_type 
operator-(const typename BitString<Alloc>::const_pointer &a,
          const typename BitString<Alloc>::const_pointer &b) {
  return ((a._p - b._p) * machine::word_bits
          + machine::clz(a._m) - machine::clz(b._m));
}

template <class Alloc>
inline bool 
BitString<Alloc>::const_pointer::operator[](
typename BitString<Alloc>::difference_type d) const {
  return *(*this + d);
}

template <class Alloc>
class BitString<Alloc>::reference {
private:
  machine::uword *_p;
  machine::uword _m;

  friend class BitString;
  reference();
  reference(machine::uword *p, machine::uword m) : _p(p), _m(m) {}

public:
  operator bool() const { return *_p & _m; }
  reference &operator= (bool b) {
    if (b)
      *_p |= machine::to_be(_m);
    else
      *_p &= machine::to_be(~_m);
  }   
  reference &operator= (const reference &r) {
    if (r)
      *_p |= machine::to_be(_m);
    else
      *_p &= machine::to_be(~_m);
  }
  pointer operator& () {
    return BitString::pointer(_p, _m);
  }
  const_pointer operator& () const {
    return BitString::const_pointer(_p, _m);
  }
};

template <class Alloc>
inline typename BitString<Alloc>::reference BitString<Alloc>::pointer::operator*() const {
  return BitString<Alloc>::reference (_p, _m);
}

template <class Alloc>
inline typename BitString<Alloc>::reference
BitString<Alloc>::pointer::operator[](typename BitString<Alloc>::difference_type d) const {
  return *(*this + d);
}

template <class Alloc>
inline typename BitString<Alloc>::iterator BitString<Alloc>::begin() {
  return BitString<Alloc>::iterator (_storage, machine::top_bit);
}
template <class Alloc>
inline typename BitString<Alloc>::const_iterator BitString<Alloc>::begin() const {
  return BitString<Alloc>::const_iterator (_storage, machine::top_bit);
}
template <class Alloc>
inline typename BitString<Alloc>::iterator BitString<Alloc>::end() {
  return begin() + _len;
}
template <class Alloc>
inline typename BitString<Alloc>::const_iterator BitString<Alloc>::end() const {
  return begin() + _len;
}
template <class Alloc>
inline typename BitString<Alloc>::reverse_iterator BitString<Alloc>::rbegin() {
  return BitString<Alloc>::reverse_iterator(end());
}
template <class Alloc>
inline typename BitString<Alloc>::const_reverse_iterator BitString<Alloc>::rbegin() const {
  return BitString<Alloc>::reverse_iterator(end());
}
template <class Alloc>
inline typename BitString<Alloc>::reverse_iterator BitString<Alloc>::rend() {
  return BitString<Alloc>::reverse_iterator(begin());
}
template <class Alloc>
inline typename BitString<Alloc>::const_reverse_iterator BitString<Alloc>::rend() const {
  return BitString<Alloc>::reverse_iterator(begin());
}

template <class Alloc>
inline typename BitString<Alloc>::reference BitString<Alloc>::at(typename BitString<Alloc>::difference_type d) {
  unsigned bytes = d / machine::word_bits;
  machine::uword mask = 1 << (machine::word_bits - 1 - d);
  return BitString<Alloc>::reference (_storage + bytes, mask);
}
template <class Alloc>
inline typename BitString<Alloc>::const_reference BitString<Alloc>::at(typename BitString<Alloc>::difference_type d) const {
  unsigned bytes = d / machine::word_bits;
  machine::uword mask = 1 << (machine::word_bits - 1 - d);
  return _storage[bytes] & machine::to_be(mask);
}

template <class Alloc>
inline typename BitString<Alloc>::reference BitString<Alloc>::operator[](typename BitString<Alloc>::difference_type d) {
  return at(d);
}
template <class Alloc>
inline typename BitString<Alloc>::const_reference BitString<Alloc>::operator[](typename BitString<Alloc>::difference_type d) const {
  return at(d);
}

template <class Alloc>
inline typename BitString<Alloc>::reference BitString<Alloc>::front() {
  return at(0);
}
template <class Alloc>
inline typename BitString<Alloc>::const_reference BitString<Alloc>::front() const {
  return at(0);
}
template <class Alloc>
inline typename BitString<Alloc>::reference BitString<Alloc>::back() {
  return at(_len - 1);
}
template <class Alloc>
inline typename BitString<Alloc>::const_reference BitString<Alloc>::back() const {
  return at(_len - 1);
}
template <class Alloc>
inline typename BitString<Alloc>::iterator BitString<Alloc>::erase(typename BitString<Alloc>::iterator position) {
  _remove (position, 1);
  return position;
}
template <class Alloc>
inline typename BitString<Alloc>::iterator BitString<Alloc>::erase (typename BitString<Alloc>::iterator s,
                                                                    typename BitString<Alloc>::iterator e) {
  return remove (s, e - s);
}

template <class Alloc>
inline typename BitString<Alloc>::iterator BitString<Alloc>::insert(typename BitString<Alloc>::iterator position,
                                                                    typename BitString<Alloc>::value_type val) {
  return _make_space (position, 1, val);
}
template <class Alloc>
inline typename BitString<Alloc>::iterator insert (typename BitString<Alloc>::iterator position, typename BitString<Alloc>::size_type n, typename BitString<Alloc>::value_type val) {
    return _make_space (position, n, val);
  }

template <class Alloc>
template <class it>
typename BitString<Alloc>::iterator BitString<Alloc>::insert (typename BitString<Alloc>::iterator position, it b, it e) {
  return insert (position, b, e, 
                 std::iterator_traits<it>::iterator_category());
}
// forward iterators can reserve space in advance
template <class Alloc>
template <class it>
typename BitString<Alloc>::iterator BitString<Alloc>::insert (typename BitString<Alloc>::iterator position, it b, it e, std::forward_iterator_tag) {
  BitString<Alloc>::iterator pos = _make_space (position, e - b);
  BitString<Alloc>::iterator ptr = pos;
  for (it i = b; i < e; ++i)
    *ptr++ = *i;
  return pos;
}
// otherwise, use a temporary BitString
template <class Alloc>
template <class it>
typename BitString<Alloc>::iterator BitString<Alloc>::insert (typename BitString<Alloc>::iterator position, it b, it e, std::input_iterator_tag) {
  BitString bs;
  bs.assign(b, e);
  return insert (position, bs.begin(), bs.end());
}

END_ASN1_NS

#endif /* ASN1_BITSTRING_H_ */
