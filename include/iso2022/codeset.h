/* Emacs, this is -*-C++-*- */

#ifndef ISO2022_CODESET_H_
#define ISO2022_CODESET_H_

#include "base.h"

#include <string>
#include <vector>

BEGIN_ISO2022_NS

typedef enum {
  C0 = 0,       // C0 control set
  C1 = 1,       // C1 control set
  G94 = 2,      // 94-character graphic
  G96 = 3,      // 96-character graphic
  M = 4,        // Multicharacter graphic
  wSR = 5,      // With standard return (ESC \x25 \x40)
  woSR = 6,     // Without standard return (one-way ticket)
} codeset_type;

/* I have no idea what drove ISO to commit this stupidity; if you're going
   to number codesets, fine, but then WHY ADD HYPHENS?!  Anyway, we encode
   8-1 as 8, 8-2 as 254, 9-1 as 9 and 9-2 as 255.  */
const unsigned CODESET_8_1 = 8;
const unsigned CODESET_8_2 = 254;
const unsigned CODESET_9_1 = 9;
const unsigned CODESET_9_2 = 255;

/* A DRCS codeset has its top nybble set to 8, and the other nybbles
   are copied from the bottom nybbles of the intermediate bytes.  (We
   limit the number of intermediate bytes to 7, which is more than
   enough.) */
const unsigned CODESET_DRCS = 0x80000000;

class codeset
{
private:
  unsigned _refcount;

public:
  codeset() : _refcount(1) {}
  virtual ~codeset() {}

  void retain() { ++_refcount; }
  void release() { if (!--_refcount) delete this; }

  virtual unsigned registration_number() const = 0;
  virtual codeset_type type() const = 0;
};

class graphic_codeset : public codeset
{
public:
  // Appends UTF-16 for 'c' to out
  virtual void decode (unsigned char c, std::u16string &out) = 0;

  /* This optional method is triggered:

     - On a codeset switch via LS0, LS1, SS2 or SS3
     - Immediately before an ESC character
     - At the end of the string

     Its purpose is to allow multibyte codesets to emit U+FFFD and reset
     their internal state if there is an incomplete multibyte sequence. */
  virtual void finish (std::u16string &out) { (void)out; }

  // Generate the requisite escape sequence to invoke this set
  virtual void invoke (code_element elt, std::string &out) = 0;

  /* Given a combining sequence at pcs, extending until pcsend, append
     encoded characters to the string out.  Returns true if the sequence
     could be encoded, false otherwise. */
  virtual bool encode (const char16_t *&pcs,
                       const char16_t *pcsend,
                       unsigned char base,
                       std::string &out) = 0;
};

class control_codeset : public codeset
{
public:
  // Decodes a control character (return -1 for no equivalent)
  virtual int decode (unsigned char c) const = 0;

  // Encodes a control character (return -1 for no equivalent)
  virtual int encode (unsigned char c) const = 0;
};

class docs_codeset : public codeset
{
public:
  /* Decodes the characters at ptr until either end of string or ESC 2/5 4/0,
     *if* supported by the coding system in question.  The escape sequence, if
     any, is eaten by the decoder, and the results are appended to out. */
  virtual void decode (const char *&ptr,
                       const char *end,
                       std::u16string &out) = 0;
};

class codeset_factory
{
public:
  static codeset_factory &builtin();

  virtual codeset *get_codeset (unsigned number) = 0;
  virtual std::vector<unsigned> graphic_codesets() const = 0;
  virtual std::vector<unsigned> control_codesets() const = 0;
};

END_ISO2022_NS

#endif
