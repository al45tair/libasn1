/* Emacs, this is -*-C++-*- */

#ifndef ISO2022_DECODER_H_
#define ISO2022_DECODER_H_

#include "base.h"
#include "codeset.h"

#include <string>

BEGIN_ISO2022_NS

class decoder
{
private:
  code_element gl, gr;
  control_codeset *c[2];
  graphic_codeset *g[4];

  typedef enum {
    NORMAL,
    SINGLE_SHIFT,
    ESCAPE,
    ANNOUNCE_CODE_STRUCTURE,
    C0_DESIGNATE,
    C1_DESIGNATE,
    SINGLE_CONTROL_FUNCTION,
    DESIGNATE_MULTIBYTE,
    G0_DESIGNATE_MULTIBYTE_94_SET_OLD, // 4/0, 4/1 and 4/2 only
    G0_DESIGNATE_MULTIBYTE_94_SET,
    G1_DESIGNATE_MULTIBYTE_94_SET,
    G2_DESIGNATE_MULTIBYTE_94_SET,
    G3_DESIGNATE_MULTIBYTE_94_SET,
    G1_DESIGNATE_MULTIBYTE_96_SET,
    G2_DESIGNATE_MULTIBYTE_96_SET,
    G3_DESIGNATE_MULTIBYTE_96_SET,
    DESIGNATE_OTHER_CODING_SYSTEM,
    DESIGNATE_OTHER_CODING_SYSTEM_W_SR,
    DESIGNATE_OTHER_CODING_SYSTEM_WO_SR,
    IDENTIFY_REVISED_REGISTRATION,
    G0_DESIGNATE_94_SET,
    G1_DESIGNATE_94_SET,
    G2_DESIGNATE_94_SET,
    G3_DESIGNATE_94_SET,
    G0_DESIGNATE_94_SET_2,
    G1_DESIGNATE_94_SET_2,
    G2_DESIGNATE_94_SET_2,
    G3_DESIGNATE_94_SET_2,
    G1_DESIGNATE_96_SET,
    G2_DESIGNATE_96_SET,
    G3_DESIGNATE_96_SET,
    G0_DESIGNATE_MULTIBYTE_94_SET_DRCS,
    G1_DESIGNATE_MULTIBYTE_94_SET_DRCS,
    G2_DESIGNATE_MULTIBYTE_94_SET_DRCS,
    G3_DESIGNATE_MULTIBYTE_94_SET_DRCS,
    G1_DESIGNATE_MULTIBYTE_96_SET_DRCS,
    G2_DESIGNATE_MULTIBYTE_96_SET_DRCS,
    G3_DESIGNATE_MULTIBYTE_96_SET_DRCS,
    G0_DESIGNATE_94_SET_DRCS,
    G1_DESIGNATE_94_SET_DRCS,
    G2_DESIGNATE_94_SET_DRCS,
    G3_DESIGNATE_94_SET_DRCS,
    G1_DESIGNATE_96_SET_DRCS,
    G2_DESIGNATE_96_SET_DRCS,
    G3_DESIGNATE_96_SET_DRCS,
  } parse_state;

  parse_state state;
  unsigned char docs_flag;
  bits mode;
  single_shift_area ssarea;

  unsigned drcs_count;
  unsigned drcs_id;
  code_element shifted_save;

  codeset_factory &cf;

  control_codeset *ic[2];
  graphic_codeset *ig[4];
  code_element igl, igr;
  unsigned flags;

public:
  decoder(codeset_factory &cset_factory,
          bits m=EIGHT_BIT,
          const unsigned control[2] = default_control_set,
          const unsigned graphic[4] = default_graphic_set,
          code_element gl=ELEMENT_G0, 
          code_element gr=ELEMENT_G1,
          single_shift_area ssa=SINGLE_SHIFT_AREA_DEFAULT,
          unsigned flgs=ALLOW_ESCAPES|ALLOW_CONTROL_CHARS);
  ~decoder();

  void reset();

  std::u16string decode(const char *ptr, size_t len);

  std::u16string decode(const std::string &iso2022) {
    return decode (iso2022.data(), iso2022.length());
  }
};

END_ISO2022_NS

#endif
