/* Emacs, this is -*-C++-*- */

#ifndef ASN1_BASE_H_
#define ASN1_BASE_H_

#define BEGIN_ASN1_NS namespace asn1 {
#define END_ASN1_NS   }

#include "machine.h"

BEGIN_ASN1_NS

typedef unsigned char      octet;
typedef machine::int8      int8;
typedef machine::uint8     uint8;
typedef machine::int16     int16;
typedef machine::uint16    uint16;
typedef machine::int32     int32;
typedef machine::uint32    uint32;
typedef machine::int64     int64;
typedef machine::uint64    uint64;

END_ASN1_NS

#endif /* ASN1_BASE_H_ */
