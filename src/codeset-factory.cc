#include <iso2022/codeset.h>
#include "codesets.h"

using namespace iso2022;

// Codesets required for ASN.1 (since they are explicitly mentioned)
//
// x 1    C0    ASCII C0
// x 6    94    ASCII
// x 13   94    Katakana
// x 72   94    Third Supplementary Set of Mosaic Characters (replaced by 173)
// x 73   C1    Attribute Control Set for Videotex (C1)
// 87   M     Japanese Graphic Character Set for Information Interchange
// x 89   94    7-bit Coded Arabic Character Set for Information Interchange
// x 102  94    Teletex Primary Set of Graphic Characters
// x 103  94    Teletex Supplementary Set of Graphic Characters
// 106  C0    Teletex Primary Set of Control Functions
// 107  C1    Teletex Supplementary Set of Control Functions
// 108  wSR   NAPLPS
// 126  96    Right-hand Part of the Latin/Greek Alphabet (replaced by 227)
// 128  94    Supplementary Set of Graphic Characters for CCITT T.101, DS3 (G2)
// 129  96    Supplementary Set of Graphic Characters for CCITT T.101, DS3 (G3)
// 144  96    Cyrillic part of the Latin/Cyrillic Alphabet (ISO 8859-5)
// 150  94    Greek Primary Set of Graphic Characters
// 153  96    Basic Cyrillic Character Set for 8-bit Codes
// 156  96    Supplementary Set of ISO/IEC 6937:1992
// 164  96    Hebrew Supplementary Set of Graphic Characters
// 165  M     Codes of the Chinese graphic character set for communication
// 168  M     Japanese Graphic Character Set for Information Interchange

namespace {

  class builtin_codeset_factory : public codeset_factory
  {
  public:
    codeset *get_codeset (unsigned number);
    std::vector<unsigned> graphic_codesets() const;
    std::vector<unsigned> control_codesets() const;
  };

  codeset *
  builtin_codeset_factory::get_codeset (unsigned number)
  {
    switch (number) {
    case   1: return new builtin::standard_c0();
    case   6: return new builtin::ascii();
    case  13: return new builtin::katakana();
    case  72: case 173: return new builtin::videotex_173();
    case  73: return new builtin::videotex_attr();
    case  77: return new builtin::standard_c1();
    case  89: return new builtin::arabic();
    case 100: return new builtin::iso_8859_1();
    case 102: return new builtin::teletex();
    case 103: return new builtin::teletex_supp();
    case 203: return new builtin::iso_8859_15();
    case 196: return new builtin::utf_8(196, wSR);
    case 190:
    case 191:
    case 192:
      return new builtin::utf_8(number, woSR);
    case 193:
    case 194:
    case 195:
      return new builtin::utf_16(number);
    case 162:
    case 174:
    case 176:
      return new builtin::ucs_2(number);
    case 163:
    case 175:
    case 177:
      return new builtin::ucs_4(number);
      
    }

    return nullptr;
  }

  std::vector<unsigned>
  builtin_codeset_factory::graphic_codesets() const
  {
    static const unsigned codesets[] = {
      203, // Prefer ISO-8859-15
      100, // then ISO-8859-1
      6, 13, 72, 173, 89, 102, 103
    };

    return std::vector<unsigned>(&codesets[0],
                                 &codesets[0] 
                                 + sizeof(codesets)/sizeof(unsigned));
  }

  std::vector<unsigned>
  builtin_codeset_factory::control_codesets() const
  {
    static const unsigned codesets[] = {
      1, 77, 73
    };

    return std::vector<unsigned>(&codesets[0],
                                 &codesets[0] 
                                 + sizeof (codesets)/sizeof (unsigned));
  }
}

codeset_factory &
codeset_factory::builtin()
{
  static builtin_codeset_factory *factory;

  if (!factory)
    factory = new builtin_codeset_factory();

  return *factory;
}
