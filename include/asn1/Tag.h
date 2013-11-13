/* Emacs, this is -*-C++-*- */

#ifndef ASN1_TAGS_H_
#define ASN1_TAGS_H_

#include "base.h"

#include <iostream>
#include <initializer_list>

BEGIN_ASN1_NS

typedef enum {
  UNIVERSAL = 0,
  APPLICATION = 1,
  CONTEXT_SPECIFIC = 2,
  PRIVATE = 3
} TagClass;

typedef enum {
  PRIMITIVE = 0,
  CONSTRUCTED = 1
} PrimitiveOrConstructed;

class Tag {
public:
  TagClass tagClass;
  uint32   number;
  
  constexpr Tag() : tagClass(UNIVERSAL), number(0) {}
  constexpr Tag(const Tag &t) : tagClass(t.tagClass), number(t.number) {}
  constexpr Tag(TagClass tc, uint32 num)
    : tagClass(tc), number(num) {}
};

inline bool operator<(Tag a, Tag b) {
  return (a.tagClass < b.tagClass 
          || (a.tagClass == b.tagClass && a.number < b.number));
}
inline bool operator<=(Tag a, Tag b) {
  return (a.tagClass <= b.tagClass 
          || (a.tagClass == b.tagClass && a.number <= b.number));
}
inline bool operator>(Tag a, Tag b) {
  return (a.tagClass > b.tagClass 
          || (a.tagClass == b.tagClass && a.number > b.number));
}
inline bool operator>=(Tag a, Tag b) {
  return (a.tagClass >= b.tagClass 
          || (a.tagClass == b.tagClass && a.number >= b.number));
}
inline bool operator==(Tag a, Tag b) {
  return a.tagClass == b.tagClass && a.number == b.number;
}
inline bool operator!=(Tag a, Tag b) {
  return a.tagClass != b.tagClass || a.number != b.number;
}

std::ostream &operator<<(std::ostream &os, Tag t);

// UNIVERSAL tags defined by X.680
constexpr Tag tBoolean = { UNIVERSAL, 1 };          // 8.6 / 18
constexpr Tag tInteger = { UNIVERSAL, 2 };          // 8.6 / 19
constexpr Tag tBitString = { UNIVERSAL, 3 };        // 8.6 / 22
constexpr Tag tOctetString = { UNIVERSAL, 4 };      // 8.6 / 23
constexpr Tag tNull = { UNIVERSAL, 5 };             // 8.6 / 24
constexpr Tag tOID = { UNIVERSAL, 6 };              // 8.6 / 32
constexpr Tag tObjectDescriptor = { UNIVERSAL, 7 }; // 8.6 / 48
constexpr Tag tExternal = { UNIVERSAL, 8 };         // 8.6 / 37
constexpr Tag tInstanceOf = tExternal;
constexpr Tag tReal = { UNIVERSAL, 9 };             // 8.6 / 21
constexpr Tag tEnumerated = { UNIVERSAL, 10 };      // 8.6 / 20
constexpr Tag tEmbeddedPDV = { UNIVERSAL, 11 };     // 8.6 / 36
constexpr Tag tUTF8String = { UNIVERSAL, 12 };      // 41.1 / 41.16
constexpr Tag tRelativeOID = { UNIVERSAL, 13 };     // 8.6 / 33
constexpr Tag tTime = { UNIVERSAL, 14 };            // 8.6 / 38
constexpr Tag tSequence = { UNIVERSAL, 16 };        // 8.6 / 25 / 26
constexpr Tag tSequenceOf = tSequence;
constexpr Tag tSet = { UNIVERSAL, 17 };             // 8.6 / 27 / 28
constexpr Tag tSetOf = tSet;
constexpr Tag tNumericString = { UNIVERSAL, 18 };   // 41.1 / 41.2
constexpr Tag tPrintableString = { UNIVERSAL, 19 }; // 41.1 / 41.4
constexpr Tag tTeletexString = { UNIVERSAL, 20 };   // 41.1
constexpr Tag tT61String = tTeletexString;          // 41.1
constexpr Tag tVideotexString = { UNIVERSAL, 21 };  // 41.1
constexpr Tag tIA5String = { UNIVERSAL, 22 };       // 41.1
constexpr Tag tUTCTime = { UNIVERSAL, 23 };         // 47
constexpr Tag tGeneralizedTime = { UNIVERSAL, 24 }; // 46
constexpr Tag tGraphicString = { UNIVERSAL, 25 };   // 41.1
constexpr Tag tVisibleString = { UNIVERSAL, 26 };   // 41.1
constexpr Tag tISO646String = tVisibleString;       // 41.1
constexpr Tag tGeneralString = { UNIVERSAL, 27 };   // 41.1
constexpr Tag tUniversalString = { UNIVERSAL, 28 }; // 41.1 / 41.6
constexpr Tag tBMPString = { UNIVERSAL, 30 };       // 41.1 / 41.15
constexpr Tag tDate = { UNIVERSAL, 31 };            // 38.4.1
constexpr Tag tTimeOfDay = { UNIVERSAL, 32};        // 38.4.2
constexpr Tag tDateTime = { UNIVERSAL, 33 };        // 38.4.3
constexpr Tag tDuration = { UNIVERSAL, 34 };        // 38.4.4
constexpr Tag tOIDIRI = { UNIVERSAL, 35 };          // 8.6 / 34
constexpr Tag tRelativeOIDIRI = { UNIVERSAL, 36 };  // 8.6 / 35

// UNIVERSAL tags defined by X.690
constexpr Tag tEndOfContents = { UNIVERSAL, 0 };    // 8.1.5

END_ASN1_NS

#endif /* ASN1_TAGS_H_ */
