/* Emacs, this is -*-C++-*- */

#ifndef CODESETS_H_
#define CODESETS_H_

#include <iso2022/codeset.h>

#define SIMPLE_GRAPHIC(n, name, t)                      \
  class name : public graphic_codeset                   \
  {                                                     \
  public:                                               \
    unsigned registration_number() const { return n; }  \
    codeset_type type() const { return t; }             \
    void decode (unsigned char c, std::u16string &out); \
    void invoke (code_element elt, std::string &out);   \
    bool encode (const char16_t *&pcs,                  \
                 const char16_t *pcsend,                \
                 unsigned char base,                    \
                 std::string &out);                     \
  }

#define CONTROL(n, name, t)                             \
  class name : public control_codeset                   \
  {                                                     \
  public:                                               \
    unsigned registration_number() const { return n; }  \
    codeset_type type() const { return t; }             \
    int encode (unsigned char c) const;                 \
    int decode (unsigned char c) const;                 \
  }

#define DOCS(n, name, t)                                \
  class name : public docs_codeset                      \
  {                                                     \
  public:                                               \
    unsigned registration_number() const { return n; }  \
    codeset_type type() const { return t; }             \
    void decode (const char *&ptr,                      \
                 const char *end,                       \
                 std::u16string &out);                  \
  }

namespace iso2022 {
  namespace builtin {
    CONTROL(1, standard_c0, C0);

    SIMPLE_GRAPHIC(6, ascii, G94);
    SIMPLE_GRAPHIC(13, katakana, G94);
    SIMPLE_GRAPHIC(173, videotex_173, G94);
    SIMPLE_GRAPHIC(89, arabic, G94);
    SIMPLE_GRAPHIC(102, teletex, G94);
    SIMPLE_GRAPHIC(103, teletex_supp, G94);

    CONTROL(73, videotex_attr, C1);
    CONTROL(77, standard_c1, C1);

    SIMPLE_GRAPHIC(100, iso_8859_1, G96);
    SIMPLE_GRAPHIC(203, iso_8859_15, G96);

    class utf_8 : public docs_codeset
    {
    private:
      unsigned     _number;
      codeset_type _type;

    public:
      utf_8 (unsigned n, codeset_type t) : _number(n), _type(t) {}

      unsigned registration_number() const { return _number; }
      codeset_type type() const { return _type; }
      void decode(const char *&ptr,
                  const char *end,
                  std::u16string &out);
    };

    class utf_16 : public docs_codeset
    {
    private:
      unsigned _number;
  
    public:
      utf_16 (unsigned n) : _number(n) {}

      unsigned registration_number() const { return _number; }
      codeset_type type() const { return woSR; }
      void decode(const char *&ptr,
                  const char *end,
                  std::u16string &out);
    };

    class ucs_2 : public docs_codeset
    {
    private:
      unsigned _number;
  
    public:
      ucs_2 (unsigned n) : _number(n) {}

      unsigned registration_number() const { return _number; }
      codeset_type type() const { return woSR; }
      void decode(const char *&ptr,
                  const char *end,
                  std::u16string &out);
    };

    class ucs_4 : public docs_codeset
    {
    private:
      unsigned _number;
  
    public:
      ucs_4 (unsigned n) : _number(n) {}

      unsigned registration_number() const { return _number; }
      codeset_type type() const { return woSR; }
      void decode(const char *&ptr,
                  const char *end,
                  std::u16string &out);
    };

  };
};

#endif
