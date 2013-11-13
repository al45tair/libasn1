/* Emacs, this is -*-C++-*- */

#ifndef ASN1_STRINGS_H_
#define ASN1_STRINGS_H_

#include <iso2022/iso2022.h>
#include "base.h"

#include <string>

BEGIN_ASN1_NS

/* Conversion helper functions

   "system" encoding means whatever the system thinks is the current
   multi-byte encoding scheme.  e.g. on Windows, the current ANSI code page,
   or on UNIX whatever langinfo(CODESET) returns.

   Obviously this can change if you do e.g. setlocale(). */
std::string utf16_to_system (const char16_t *ptr, size_t len);
inline std::string utf16_to_system (const std::u16string &s) {
  return utf16_to_system (s.data(), s.length());
}
std::u16string system_to_utf16 (const char *ptr, size_t len);
inline std::u16string system_to_utf16 (const std::string &s) {
  return system_to_utf16 (s.data(), s.length());
}

std::string utf8_to_system (const char *ptr, size_t len);
inline std::string utf8_to_system (const std::string &s) {
  return utf8_to_system (s.data(), s.length());
}
std::string system_to_utf8 (const char *ptr, size_t len);
inline std::string system_to_utf8 (const std::string &s) {
  return system_to_utf8 (s.data(), s.length());
}

std::u16string utf8_to_utf16 (const char *ptr, size_t len);
inline std::u16string utf8_to_utf16 (const std::string &s) {
  return utf8_to_utf16 (s.data(), s.length());
}
std::string utf16_to_utf8 (const char16_t *ptr, size_t len);
inline std::string utf16_to_utf8 (const std::u16string &s) {
  return utf16_to_utf8 (s.data(), s.length());
}

/*
 * In this ASN.1 library, strings remain as their encoded types and
 * in encoded representation.  If you wish to convert them to localised
 * or Unicode strings, you may do so, but the library will not do this
 * when encoding or decoding.
 *
 * The benefits of this are:
 *
 *   - The encoder and decoder will never lose information silently
 *   - You can always compare strings, without encodings getting in the way
 *   - You are reminded when encoding that you need to specify the string
 *     type
 *
 * To support this, we declare a number of subclasses of std::basic_string.
 * We use subclassing because this means the different strings are of
 * different actual types.
 *
 * We provide constructors and explicit conversion operators to convert
 * between these types and std::u16string.  We do *not* provide conversions
 * to or from std::string by default, since it is of unknown encoding,
 * though we provide the system_to_utf16(), utf16_to_system(), utf8_to_utf16()
 * and utf16_to_utf8() helper functions to make life easier for users.
 *
 * Also, note the following: we raise exceptions if you try to set one of
 * these strings to something it doesn't support.  If, however, it already
 * contains such an item (e.g. because it has been read from the wire),
 * you'll instead get U+FFFD characters when decoding it.
 *
 */

#define DECLARE_STRING(name, ctype)                             \
template <class Alloc = std::allocator<ctype> >                 \
class name : public std::basic_string<ctype,                    \
                                      std::char_traits<ctype>,  \
                                      Alloc>

#define STANDARD_STRING_CONSTRUCTORS(name, ctype)                       \
private:                                                                \
  typedef std::basic_string<ctype, std::char_traits<ctype>, Alloc>      \
    superclass;                                                         \
public:                                                                 \
  explicit name (const Alloc &alloc = Alloc())                          \
    : superclass(alloc) {}                                       \
  name (const name &str)                                                \
    : superclass(str) {}                                         \
  name (const name &str, size_t pos, size_t len,                        \
        const Alloc &alloc = Alloc())                                   \
    : superclass(str, pos, len, alloc) {}                        \
  name (const ctype *s,                                                 \
        const Alloc &alloc = Alloc())                                   \
    : superclass(s, alloc) {}                                    \
  name (const ctype *s, size_t n,                                       \
        const Alloc &alloc = Alloc())                                   \
    : superclass(s, n, alloc) {}                                 \
  name (size_t n, ctype c,                                              \
        const Alloc &alloc = Alloc())                                   \
    : superclass(n, c, alloc) {}                                 \
  template <class InputIterator>                                        \
  name (InputIterator first, InputIterator last,                        \
        const Alloc &alloc = Alloc())                                   \
    : superclass(first, last, alloc) {}

DECLARE_STRING(basic_bmp_string, char16_t)
{
protected:
  void check_ucs2(const char16_t *ptr, size_t length) {
    const char16_t *end = ptr + length;
    while (ptr < end) {
      if (*ptr >= 0xd800 || *ptr <= 0xdfff)
        throw std::runtime_error("BMP strings cannot contain surrogates");
      ++ptr;
    }
  }

public:
  STANDARD_STRING_CONSTRUCTORS(basic_bmp_string, char16_t);

  basic_bmp_string(const std::u16string &s,
                   const Alloc &alloc = Alloc())
    : superclass(alloc) {
    check_ucs2 (s.data(), s.length());
    this->assign (s.data(), s.length());
  }

  explicit operator std::u16string () const {
    return std::u16string(this->data(), this->length());
  }
};

DECLARE_STRING(basic_general_string, char)
{
protected:
  std::string from_utf16 (const std::u16string &s) {
    iso2022::codeset_factory &cf = iso2022::codeset_factory::builtin();
    iso2022::encoder encoder(cf);
    encoder.set_permitted_graphic_codesets (cf.graphic_codesets());

    return encoder.encode(s);
  }

public:
  STANDARD_STRING_CONSTRUCTORS(basic_general_string, char);

  basic_general_string(const std::u16string &s,
                       const Alloc &alloc = Alloc())
    : superclass(alloc) {
    std::string tmp = from_utf16(s);
    this->assign (tmp.begin(), tmp.end());
  }
  basic_general_string(const char16_t *s,
                       const Alloc &alloc = Alloc())
    : superclass(alloc) {
    std::u16string utf16(s);
    std::string tmp = from_utf16(s);
    this->assign (tmp.begin(), tmp.end());
  }

  explicit operator std::u16string() const {
    iso2022::decoder decoder(iso2022::codeset_factory::builtin());
    
    return decoder.decode(this->data(), this->length());
  }
};

DECLARE_STRING(basic_graphic_string, char)
{
protected:
  std::string from_utf16 (const std::u16string &s) {
    iso2022::codeset_factory &cf = iso2022::codeset_factory::builtin();
    iso2022::encoder encoder(cf);
    encoder.set_permitted_graphic_codesets (cf.graphic_codesets());

    return encoder.encode(s);
  }

public:
  STANDARD_STRING_CONSTRUCTORS(basic_graphic_string, char);

  basic_graphic_string(const std::u16string &s,
                       const Alloc &alloc = Alloc())
    : superclass(alloc) {
    std::string tmp = from_utf16(s);
    this->assign (tmp.begin(), tmp.end());
  }
  basic_graphic_string(const char16_t *s,
                       const Alloc &alloc = Alloc())
    : superclass(alloc) {
    std::u16string utf16(s);
    std::string tmp = from_utf16(s);
    this->assign (tmp.begin(), tmp.end());
  }

  explicit operator std::u16string() const {
    iso2022::codeset_factory &cf = iso2022::codeset_factory::builtin();
    iso2022::decoder decoder(cf, iso2022::EIGHT_BIT,
                             iso2022::default_control_set,
                             iso2022::default_graphic_set,
                             iso2022::ELEMENT_G0,
                             iso2022::ELEMENT_G1,
                             iso2022::SINGLE_SHIFT_AREA_DEFAULT,
                             iso2022::ALLOW_ESCAPES);
    
    return decoder.decode(this->data(), this->length());
  }
};

DECLARE_STRING(basic_ia5_string, char)
{
public:
  STANDARD_STRING_CONSTRUCTORS(basic_ia5_string, char);

  basic_ia5_string(const std::u16string &s,
                   const Alloc &alloc = Alloc())
    : superclass(alloc) {
    for (auto ptr = s.begin(); ptr < s.end(); ++ptr)
      if (*ptr > 0x7f)
        throw std::runtime_error("bad character in IA5 string");
    this->assign (s.begin(), s.end());
  }
  basic_ia5_string(const char16_t *s,
                   const Alloc &alloc = Alloc())
    : superclass(alloc) {
    const char16_t *ptr = s;
    while (*ptr) {
      if (*ptr > 0x7f)
        throw std::runtime_error("bad character in IA5 string");
      ++ptr;
    }
    this->assign (s, ptr);
  }

  explicit operator std::u16string() const {
    return std::u16string(this->begin(), this->end());
  }
};

DECLARE_STRING(basic_numeric_string, char)
{
public:
  STANDARD_STRING_CONSTRUCTORS(basic_numeric_string, char);

  basic_numeric_string(const std::u16string &s,
                       const Alloc &alloc = Alloc())
    : superclass(alloc) {
    for (auto ptr = s.begin(); ptr < s.end(); ++ptr)
      if (*ptr != ' ' && (*ptr < '0' || *ptr > '9'))
        throw std::runtime_error("bad character in numeric string");
    this->assign (s.begin(), s.end());
  }
  basic_numeric_string(const char16_t *s,
                   const Alloc &alloc = Alloc())
    : superclass(alloc) {
    const char16_t *ptr = s;
    while (*ptr) {
      if (*ptr != ' ' && (*ptr < '0' || *ptr > '9'))
        throw std::runtime_error("bad character in numeric string");
      ++ptr;
    }
    this->assign (s, ptr);
  }

  explicit operator std::u16string() const {
    return std::u16string(this->begin(), this->end());
  }
};

DECLARE_STRING(basic_printable_string, char)
{
protected:
  template <class charT, class InputIterator>
  void check_printable(InputIterator p, InputIterator e) {
    while (p < e) {
      charT ch = *p;

      if ((ch < 'A' || ch > 'Z')
          && (ch < 'a' || ch > 'z')
          && (ch < '0' || ch > '9')) {
        switch (ch) {
        case ' ': case '\'': case '(': case ')':
        case '+': case ',': case '-': case '.':
        case '/': case ':': case '=': case '?':
          break;
        default:
          throw std::runtime_error("bad character in printable string");
        }
      }
      ++p;
    }
  }

public:
  STANDARD_STRING_CONSTRUCTORS(basic_printable_string, char);

  basic_printable_string(const std::u16string &s,
                         const Alloc &alloc = Alloc())
    : superclass(alloc) {
    check_printable<char16_t> (s.begin(), s.end());
    this->assign (s.begin(), s.end());
  }
  basic_printable_string(const char16_t *s,
                         const Alloc &alloc = Alloc())
    : superclass(alloc) {
    const char16_t *end = s;
    while (*end)
      ++end;
    check_printable<char16_t> (s, end);
    this->assign (s, end);
  }

  explicit operator std::u16string() const {
    return std::u16string(this->begin(), this->end());
  }
};

extern const unsigned t61_graphic_codesets[];
extern const unsigned t61_graphic_codeset_count;
extern const unsigned t61_default_graphic_set[], t61_default_control_set[];

DECLARE_STRING(basic_t61_string, char)
{
protected:
  std::string from_utf16 (const std::u16string &s) {
    iso2022::encoder encoder(iso2022::codeset_factory::builtin(),
                             iso2022::EIGHT_BIT,
                             t61_default_control_set,
                             t61_default_graphic_set);
    encoder.set_permitted_graphic_codesets (t61_graphic_codesets,
                                            t61_graphic_codeset_count);

    return encoder.encode(s);
  }

public:
  STANDARD_STRING_CONSTRUCTORS(basic_t61_string, char);

  basic_t61_string(const std::u16string &s,
                   const Alloc &alloc = Alloc())
    : superclass(alloc) {
    std::string tmp = from_utf16(s);
    this->assign (tmp.begin(), tmp.end());
  }
  basic_t61_string(const char16_t *s,
                   const Alloc &alloc = Alloc())
    : superclass(alloc) {
    std::u16string utf16(s);
    std::string tmp = from_utf16(s);
    this->assign (tmp.begin(), tmp.end());
  }

  explicit operator std::u16string() const {
    iso2022::decoder decoder(iso2022::codeset_factory::builtin(),
                             iso2022::EIGHT_BIT,
                             t61_default_control_set,
                             t61_default_graphic_set);
    
    return decoder.decode(this->data(), this->length());
  }
};

DECLARE_STRING(basic_universal_string, char32_t)
{
public:
  STANDARD_STRING_CONSTRUCTORS(basic_universal_string, char32_t);

  basic_universal_string(const std::u16string &s,
                         const Alloc &alloc = Alloc())
    : superclass(alloc) {
    auto ptr = s.begin(), end = s.end();
    while (ptr < end) {
      char32_t ch = *ptr++;
      if (ch >= 0xd800 && ch <= 0xdbff) {
        char16_t ch2 = *ptr++;
        ch = 0x10000 + (((ch & 0x3ff) << 10) | (ch2 & 0x3ff));
      }
      this->push_back (ch);
    }
  }
  basic_universal_string(const char16_t *s,
                         const Alloc &alloc = Alloc())
    : superclass(alloc) {
    const char16_t *ptr = s;
    while (*ptr) {
      char32_t ch = *ptr++;
      if (ch >= 0xd800 && ch <= 0xdbff) {
        char16_t ch2 = *ptr++;
        ch = 0x10000 + (((ch & 0x3ff) << 10) | (ch2 & 0x3ff));
      }
      this->push_back (ch);
    }
  }

  explicit operator std::u16string() const {
    std::u16string r;
    for (auto ptr = this->begin(); ptr < this->end(); ++ptr) {
      char32_t ch = *ptr;

      if (ch >= 0xd800 && ch <= 0xdfff)
        r.push_back (0xfffd);
      else if (ch > 0x10000) {
        ch -= 0x10000;
        r.push_back (0xd800 | ((ch >> 10) & 0x3ff));
        r.push_back (0xdc00 | (ch & 0x3ff));
      }
    }

    return r;
  }
};

DECLARE_STRING(basic_utf8_string, char)
{
public:
  STANDARD_STRING_CONSTRUCTORS(basic_utf8_string, char);

  basic_utf8_string(const std::u16string &s,
                    const Alloc &alloc = Alloc())
    : superclass(alloc) {
    std::string utf8 = utf16_to_utf8 (s.data(), s.length());
    this->assign (utf8.begin(), utf8.end());
  }
  basic_utf8_string(const char16_t *s,
                    const Alloc &alloc = Alloc())
    : superclass(alloc) {
    const char16_t *end = s; 

    while (*end)
      ++end;

    std::string utf8 = utf16_to_utf8 (s, end - s);
    this->assign (utf8.begin(), utf8.end());
  }
  
  explicit operator std::u16string() const {
    return utf8_to_utf16(this->data(), this->length());
  }
};

extern const unsigned videotex_graphic_codesets[];
extern const unsigned videotex_graphic_codeset_count;
extern const unsigned videotex_default_graphic_set[];
extern const unsigned videotex_default_control_set[];

DECLARE_STRING(basic_videotex_string, char)
{
protected:
  std::string from_utf16 (const std::u16string &s) {
    iso2022::encoder encoder(iso2022::codeset_factory::builtin(),
                             iso2022::EIGHT_BIT,
                             videotex_default_control_set,
                             videotex_default_graphic_set);
    encoder.set_permitted_graphic_codesets (videotex_graphic_codesets,
                                            videotex_graphic_codeset_count);

    return encoder.encode(s);
  }

public:
  STANDARD_STRING_CONSTRUCTORS(basic_videotex_string, char);

  basic_videotex_string(const std::u16string &s,
                   const Alloc &alloc = Alloc())
    : superclass(alloc) {
    std::string tmp = from_utf16(s);
    this->assign (tmp.begin(), tmp.end());
  }
  basic_videotex_string(const char16_t *s,
                        const Alloc &alloc = Alloc())
    : superclass(alloc) {
    std::u16string utf16(s);
    std::string tmp = from_utf16(s);
    this->assign (tmp.begin(), tmp.end());
  }

  explicit operator std::string() const {
    return utf16_to_system((std::u16string)*this);
  }

  explicit operator std::u16string() const {
    iso2022::decoder decoder(iso2022::codeset_factory::builtin(),
                             iso2022::EIGHT_BIT,
                             videotex_default_control_set,
                             videotex_default_graphic_set);
    
    return decoder.decode(this->data(), this->length());
  }
};

DECLARE_STRING(basic_visible_string, char)
{
public:
  STANDARD_STRING_CONSTRUCTORS(basic_visible_string, char);


  basic_visible_string(const std::u16string &s,
                       const Alloc &alloc = Alloc())
    : superclass(alloc) {
    for (auto ptr = s.begin(); ptr < s.end(); ++ptr) {
      char16_t ch = *ptr;
      if (ch < 0x20 || ch >= 0x7f)
        throw std::runtime_error("bad character in visible string");
    }
    this->assign (s.begin(), s.end());
  }
  basic_visible_string(const char16_t *s,
                       const Alloc &alloc = Alloc())
    : superclass(alloc) {
    const char16_t *end = s;
    while (*end) {
      char16_t ch = *end++;
      if (ch < 0x20 || ch >= 0x7f)
        throw std::runtime_error("bad character in visible string");
    }
    this->assign (s, end);
  }

  explicit operator std::u16string() const {
    return std::u16string(this->begin(), this->end());
  }
};

typedef basic_bmp_string<>       BMPString;
typedef basic_general_string<>   GeneralString;
typedef basic_graphic_string<>   GraphicString;
typedef basic_ia5_string<>       IA5String; 
typedef basic_numeric_string<>   NumericString;
typedef basic_printable_string<> PrintableString;
typedef basic_t61_string<>       T61String;
typedef T61String                TeletexString;
typedef basic_universal_string<> UniversalString;
typedef basic_utf8_string<>      UTF8String;
typedef basic_videotex_string<>  VideotexString;
typedef basic_visible_string<>   VisibleString;
typedef VisibleString            ISO646String;

END_ASN1_NS

#endif
