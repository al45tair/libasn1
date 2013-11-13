/* Emacs, this is -*-C++-*- */

#ifndef ASN1_CHOICE_H_
#define ASN1_CHOICE_H_

#include "base.h"
#include "Tag.h"
#include "traits.h"
#include "DEREncoder.h"

BEGIN_ASN1_NS

/* This is a variant type that can be used to hold any set of values that
   have distinct ASN.1 tags associated with them.  e.g.

      choice<int, bool, OID> c;

      c = my_oid;
      // t.tag() is now tOID
      c = 5;
      // t.tag() is now tInteger
      c = true;
      // t.tag() is now tBoolean

  The primary use of this type is for decoding, but it works with the
  encoder too. */

template <typename... Types>
class choice {
};

template<>
class choice<>
{
protected:
  Tag _tag;
  void *_ptr;

  void assign() {}

protected:
  choice(Tag t, void *ptr) : _tag(t), _ptr(ptr) {}

public:
  choice() : _tag(tNull), _ptr(nullptr) { }

  Tag tag() const { return _tag; }
};

template <typename Head, typename... Tail>
class choice<Head, Tail...> : public choice<Tail...>
{
protected:
  virtual void clear();

  using choice<Tail...>::assign;
  void assign(const Head &h);

public:
  choice() : choice<Tail...>() { }
  choice(const Head &h) : choice<Tail...>(traits<Head>::tag, new Head(h)) { }
  ~choice();

  template <typename T>
  choice &operator=(const T &v) {
    assign (v);
    return *this;
  }

  explicit operator Head& () const;
};

template<typename Head, typename... Tail>
choice<Head, Tail...>::~choice() {
  this->clear();
}

template<typename Head, typename... Tail>
void choice<Head, Tail...>::clear() {
  if (this->_ptr) {
    if (this->_tag == traits<Head>::tag)
      delete (Head *)this->_ptr;
    this->_ptr = nullptr;
  }
}

inline DEREncoder &operator<<(DEREncoder &e, const choice<> &c) {
  (void)e;
  (void)c;
  throw std::runtime_error("uninitialized choice");
}

template<typename Head, typename... Tail>
inline DEREncoder &operator<<(DEREncoder &e, const choice<Head, Tail...> &c) {
  if (c.tag() == traits<Head>::tag)
    return e << (Head)c;
  return e << (choice<Tail...>)c;
}

template <typename Head, typename... Tail>
inline void choice<Head, Tail...>::assign(const Head &h)
{
  this->clear();
  this->_ptr = new Head(h);
  this->_tag = traits<Head>::tag;
}

template <typename Head, typename... Tail>
inline choice<Head, Tail...>::operator Head&() const
{
  if (this->_tag != traits<Head>::tag)
    throw std::runtime_error("attempt to extract value of incorrect type "
                             "from asn1::choice object");
  return *(Head *)this->_ptr;
}

END_ASN1_NS

#endif
