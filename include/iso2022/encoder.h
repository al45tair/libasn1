/* Emacs, this is -*-C++-*- */

#ifndef ISO2022_ENCODER_H_
#define ISO2022_ENCODER_H_

#include "base.h"
#include "codeset.h"
#include <vector>

BEGIN_ISO2022_NS

class encoder
{
private:
  code_element gl, gr;
  control_codeset *c[2];
  graphic_codeset *g[4];

  unsigned last_used[4];
  unsigned clock;

  codeset_factory &cf;
  std::vector<graphic_codeset *> graphic_sets;

  code_element      igl, igr;
  control_codeset  *ic[2];
  graphic_codeset  *ig[4];

  bits              mode;
  single_shift_area ssarea;

  unsigned flags;

  static bool is_combining (char16_t c);

public:
  encoder(codeset_factory &cset_factory,
          bits bits = EIGHT_BIT,
          const unsigned control[2] = default_control_set,
          const unsigned graphic[4] = default_graphic_set,
          code_element initial_gl = ELEMENT_G0,
          code_element initial_gr = ELEMENT_G1,
          single_shift_area ssa = SINGLE_SHIFT_AREA_DEFAULT,
          unsigned flgs = CANONICAL_MODE);
  ~encoder();

  /* By default, only codeset 6 is allowed; you must specify a list of
     allowable sets, in priority order, if you want anything else */
  void set_permitted_graphic_codesets (const std::vector<unsigned> &codesets);
  void set_permitted_graphic_codesets (const unsigned *codesets, unsigned count);

  void reset();

  /* Note: ISO 2022 does not specify an equivalent to Unicode's U+FFFD.
     As a result, we use the specified replacement character instead.  Be
     certain that this will not have unwanted consequences! */
  std::string encode(const char16_t *utf16, size_t len, char replacement = '?');

  std::string encode(const std::u16string &utf16, char replacement = '?') {
    return encode(utf16.data(), utf16.length(), replacement);
  }
};

END_ISO2022_NS

#endif
