/* Emacs, this is -*-C++-*- */

#ifndef ASN1_TRAITS_H_
#define ASN1_TRAITS_H_

#include "base.h"
#include "Tag.h"
#include "BitString.h"
#include "strings.h"
#include "OID.h"

#include <vector>
#include <deque>
#include <list>
#include <map>
#include <deque>
#include <set>

BEGIN_ASN1_NS

template <typename T>
class traits
{
};

template<>
class traits<bool>
{
public:
  static const Tag tag;
};

template<>
class traits<int32>
{
public:
  static const Tag tag;
};
template<>
class traits<uint32>
{
public:
  static const Tag tag;
};
template<>
class traits<int64>
{
public:
  static const Tag tag;
};
template<>
class traits<uint64>
{
public:
  static const Tag tag;
};

template<>
class traits<double>
{
public:
  static const Tag tag;
};

template<>
class traits<OID>
{
public:
  static const Tag tag;
};

template<>
class traits<RelativeOID>
{
public:
  static const Tag tag;
};

template<>
class traits<BMPString>
{
public:
  static const Tag tag;
};
template<>
class traits<UniversalString>
{
public:
  static const Tag tag;
};
template<>
class traits<GeneralString>
{
public:
  static const Tag tag;
};
template<>
class traits<GraphicString>
{
public:
  static const Tag tag;
};
template<>
class traits<IA5String>
{
public:
  static const Tag tag;
};
template<>
class traits<NumericString>
{
public:
  static const Tag tag;
};
template<>
class traits<PrintableString>
{
public:
  static const Tag tag;
};
template<>
class traits<T61String>
{
public:
  static const Tag tag;
};
template<>
class traits<UTF8String>
{
public:
  static const Tag tag;
};
template<>
class traits<VideotexString>
{
public:
  static const Tag tag;
};
template<>
class traits<ISO646String>
{
public:
  static const Tag tag;
};

END_ASN1_NS

#endif
