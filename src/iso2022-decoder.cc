#include <iso2022/decoder.h>
#include <cstdio>

using namespace iso2022;

const unsigned iso2022::default_control_set[2] = { codesets::standard_c0, 0 };
const unsigned iso2022::default_graphic_set[4] = { codesets::ascii, 0, 0, 0 };

// Table 1 - 94-character graphic character sets (three-character escape)
static const unsigned char gtbl1[64] = {
  2,   4,   6,   8, 254,   9, 255,  10,  11,  13,  14,  21,  16,  39,  37,  38,
 53,  54,  25,  55,  57,  27,  47,  49,  31,  15,  17,  18,  19,  50,  51,  59,
 60,  61,  70,  71, 173,  68,  69,  84,  85,  86,  88,  89,  90,  91,  92,  93,
 94,  95,  96,  98,  99, 102, 103, 121, 122, 137, 141, 146, 128, 147,   0,   0
};

// Table 2 - 94-character graphic character sets (four-character escape)
static const unsigned char gtbl2[64] = {
  150, 151, 170, 207, 230, 231, 232, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Table 3 - 96-character graphic character sets (three-character escape)
static const unsigned char gtbl3[64] = {
111, 100, 101, 109, 110, 123, 126, 127, 138, 139, 142, 143, 144, 148, 152, 153,
154, 155, 156, 164, 166, 167, 157,   0, 158, 179, 180, 181, 182, 197, 198, 199,
200, 201, 203, 204, 205, 206, 226, 208, 209, 227, 234,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 129,   0,   0
};

// Table 4 - Multiple-byte graphic character sets (four-character escape)
static const unsigned char gtbl4[64] = {
 42,  58, 168, 149, 159, 165, 169, 171, 172, 183, 184, 185, 186, 187, 202, 228,
229, 233,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

// Table 5 - C0 control character sets
static const unsigned char ctbl0[64] = {
  1,   7,  48,  26,  36, 106,  74, 104, 130, 132, 134, 135, 140,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

// Table 6 - C1 control character sets
static const unsigned char ctbl1[64] = {
 56,  73, 124,  77, 133,  40, 136, 105, 107,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

// Table 8 - Coding systems different from that of ISO 2022 (three-ch esc)
static const unsigned char docs0[64] = {
  0, 108, 178, 131, 145, 160, 161, 196, 188,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

// Table 9 - Coding systems different from that of ISO 2022 (two-ch esc)
static const unsigned char docs1[64] = {
162, 163, 125, 174, 175, 176, 177, 190, 191, 192, 193, 194, 195,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

decoder::decoder(codeset_factory &cset_factory,
                 bits m,
                 const unsigned control[2],
                 const unsigned graphic[4],
                 code_element gl_elt,
                 code_element gr_elt,
                 single_shift_area ssa,
                 unsigned flgs)
  : cf(cset_factory), flags(flgs)
{
  mode = m;

  for (unsigned n = 0; n < 2; ++n)
    ic[n] = c[n] = nullptr;

  for (unsigned n = 0; n < 4; ++n)
    ig[n] = g[n] = nullptr;

  for (unsigned n = 0; n < 2; ++n) {
    if (control[n]) {
      codeset *ctl = cf.get_codeset (control[n]);

      if (!ctl) {
        char buffer[80];
        std::sprintf (buffer, "Unknown C%u code set ISO IR %d", n, control[n]);
        throw std::runtime_error(buffer);
      }

      if ((n == 0 && ctl->type() != C0)
          || (n == 1 && ctl->type() != C1)) {
        char buffer[80];
        sprintf (buffer, 
                 "Attempted to select non-C%u code set ISO IR %d into C%u",
                 n, control[n], n);
        ctl->release();
        throw std::runtime_error(buffer);
      }

      ic[n] = c[n] = (control_codeset *)ctl;
      if (ic[n])
        ic[n]->retain();
    }
  }

  for (unsigned n = 0; n < 4; ++n) {
    if (graphic[n]) {
      codeset *cset = cf.get_codeset (graphic[n]);

      if (!cset) {
        char buffer[80];
        std::sprintf (buffer, "Unknown graphic code set ISO IR %d", 
                      graphic[n]);
        throw std::runtime_error(buffer);
      }

      switch (cset->type()) {
      case G94:
      case G96:
      case M:
        break;
      case C0:
      case C1:
        {
          char buffer[80];
          std::sprintf (buffer,
                        "Attempted to select control codeset ISO IR %d into G%u",
                        graphic[n], n);
          cset->release();
          throw std::runtime_error(buffer);
        }
        break;
      case wSR:
      case woSR:
        {
          char buffer[80];
          std::sprintf (buffer,
                        "Attempted to select non-ISO 2022 codeset ISO IR %d "
                        "into G%u",
                        graphic[n], n);
          cset->release();
          throw std::runtime_error(buffer);
        }
      }

      ig[n] = g[n] = (graphic_codeset *)cset;
      if (ig[n])
        ig[n]->retain();
    }
  }

  igl = gl = gl_elt;
  igr = gr = gr_elt;

  state = NORMAL;

  if (ssa == SINGLE_SHIFT_AREA_DEFAULT) {
    if (mode == EIGHT_BIT)
      ssa = SINGLE_SHIFT_AREA_GR;
    else
      ssa = SINGLE_SHIFT_AREA_GL;
  }

  ssarea = ssa;
}

decoder::~decoder()
{
  for (unsigned n = 0; n < 2; ++n) {
    if (c[n]) {
      c[n]->release ();
      c[n] = nullptr;
    }
    if (ic[n]) {
      ic[n]->release ();
      ic[n] = nullptr;
    }
  }
  for (unsigned n = 0; n < 4; ++n) {
    if (g[n]) {
      g[n]->release();
      g[n] = nullptr;
    }
    if (ig[n]) {
      ig[n]->release();
      ig[n] = nullptr;
    }
  }
}

void
decoder::reset()
{
  gl = igl;
  gr = igr;
  for (unsigned n = 0; n < 2; ++n) {
    if (c[n])
      c[n]->release();
    c[n] = ic[n];
    if (c[n])
      c[n]->retain();
  }
  for (unsigned n = 0; n < 4; ++n) {
    if (g[n])
      g[n]->release();
    g[n] = ig[n];
    if (g[n])
      g[n]->retain();
  }
  state = NORMAL;
}

/* VERY IMPORTANT: To avoid security issues, this code MUST NOT DROP CHARACTERS
   IN THE EVENT OF AN ISO 2022 SYNTAX ERROR.  Nor must syntax errors result in
   output that could legitimately be generated.

   We either generate U+FFFD (where we can't decode a character), or for escape
   sequences we replace the ESC with U+241B (the escape *symbol*) and output
   the remainder. */
std::u16string
decoder::decode (const char *str, size_t len)
{
  const char *ptr = str;
  const char *end = ptr + len;
  std::u16string result;

  while (ptr < end) {
    unsigned char ch = *ptr;

    switch (state) {
    case SINGLE_SHIFT:
      // In SINGLE_SHIFT state, we must have a character in the single shift area
      if (ssarea == SINGLE_SHIFT_AREA_GL) {
        if (ch < 0x20 || ch > 0x7f) {
          result += u'\ufffd';
          state = NORMAL;
          continue;
        }
      } else {
        if (ch < 0xa0) {
          result += u'\ufffd';
          state = NORMAL;
          continue;
        }
      }

      // Fall through
    case NORMAL:
      ++ptr;

      if (ch == ESC) {
        if (flags & ALLOW_ESCAPES)
          state = ESCAPE;
        else
          result += u'\u241b';
        continue;
      }

      if (!(flags & ALLOW_CONTROL_CHARS)) {
        if (ch <= 0x1f) {
          if ((ch != LS0 && ch != LS1) || !(flags & ALLOW_SHIFT_CHARS)) {
            // Map low control characters to their symbols
            result += (char16_t)(0x2400 + ch);
            continue;
          }
        } else if (ch >= 0x80 && ch <= 0x9f) {
          if ((ch != SS2 && ch != SS3) || !(flags & ALLOW_SHIFT_CHARS)) {
            // Sadly there is no set of C1 symbols in Unicode at present
            result += u'\ufffd';
            continue;
          }
        }
      } else {
        int dch = -1;

        if (ch <= 0x1f && c[0]) {
          dch = c[0]->decode (ch);
        } else if (ch >= 0x80 && ch <= 0x9f && c[1]) {
          dch = c[1]->decode (ch);
        }

        if (dch < 0) {
          if (ch <= 0x1f) {
            result += (char16_t)(0x2400 + ch);  
            continue;
          } else if (ch >= 0x80 && ch <= 0x9f) {
            result += u'\ufffd';
            continue;
          }
        } else {
          ch = dch;
        }
      }

      switch (ch) {
      case LS0: 
        if (g[gl])
          g[gl]->finish (result);
        gl = ELEMENT_G0; 
        continue;
      case LS1:
        if (g[gl])
          g[gl]->finish (result);
        gl = ELEMENT_G1; 
        continue;
      case SS2:
        if (ssarea == SINGLE_SHIFT_AREA_GL) {
          shifted_save = gl;
          state = SINGLE_SHIFT;
          if (g[gl])
            g[gl]->finish (result);
          gl = ELEMENT_G2;
        } else {
          shifted_save = gr;
          state = SINGLE_SHIFT;
          if (g[gr])
            g[gr]->finish (result);
          gr = ELEMENT_G2;
        }
        continue;
      case SS3:
        if (ssarea == SINGLE_SHIFT_AREA_GL) {
          shifted_save = gl;
          state = SINGLE_SHIFT;
          if (g[gl])
            g[gl]->finish (result);
          gl = ELEMENT_G3;
        } else {
          shifted_save = gr;
          state = SINGLE_SHIFT;
          if (g[gr])
            g[gr]->finish (result);
          gr = ELEMENT_G3;
        }
        continue;
      }

      if (ch >= 0x20 && ch <= 0x7f) {
        if (!g[gl])
          result += u'\ufffd';
        else {
          if ((ch == 0x20 || ch == 0x7f) && g[gl]->type() == G94)
            result += (char16_t)ch;
          else
            g[gl]->decode (ch, result);
        }
      } else if (ch >= 0xa0) {
        if (mode != EIGHT_BIT || !g[gr])
          result += u'\ufffd';
        else
          g[gr]->decode (ch - 0x80, result);
      } else {
        // Control characters we haven't used are just appended
        result += ch;
      }

      // If we're shifted, un-shift
      if (state == SINGLE_SHIFT) {
        if (ssarea == SINGLE_SHIFT_AREA_GL)
          gl = shifted_save;
        else
          gr = shifted_save;

        state = NORMAL;
      }
      break;

    case ESCAPE:
      if (g[gl])
        g[gl]->finish (result);
      if (g[gr])
        g[gr]->finish (result);

      if (ch < 0x20 || ch >= 0x7f) {
        // This is invalid (we put in an ESC control picture)
        result += u'\u241b';
        state = NORMAL;

        // *Don't* increment ptr in this case; the next character could be valid
        continue;
      }

      if (ch >= 0x40 && ch <= 0x50) {
        // Type Fe - control function in the C1 set
        state = NORMAL;

        ch += 0x40;
        if (c[1])
          ch = c[1]->decode (ch);

        switch (ch) {
        case LS0: gl = ELEMENT_G0; ++ptr; continue;
        case LS1: gr = ELEMENT_G1; ++ptr; continue;
        case SS2:
          if (ssarea == SINGLE_SHIFT_AREA_GL) {
            shifted_save = gl;
            state = SINGLE_SHIFT;
            gl = ELEMENT_G2;
          } else {
            shifted_save = gr;
            state = SINGLE_SHIFT;
            gr = ELEMENT_G2;
          }
          ++ptr;
          continue;
        case SS3:
          if (ssarea == SINGLE_SHIFT_AREA_GL) {
            shifted_save = gl;
            state = SINGLE_SHIFT;
            gl = ELEMENT_G3;
          } else {
            shifted_save = gr;
            state = SINGLE_SHIFT;
            gr = ELEMENT_G3;
          }
          ++ptr;
          continue;
        default:
          break;
        }

        if (ch < 0x20 || (ch >= 0x80 && ch <= 0x9f)) {
          result += ch;
          ++ptr;
        } else {
          // This is invalid; it must be a control character (we put in an ESC)
          result += u'\u241b';

          // *Don't* increment ptr in this case
        }
        continue;
      }

      if (ch >= 0x60 && ch <= 0x7e) {
        /* Type Fs - Standardized single control function; with the exception
           of the few that we process, we map these to the private use area
           by adding 0xe000 to the final character. */
        state = NORMAL;

        switch (ch) {
        case 0x60: // ISO IR 32  - Disable Manual Input
        case 0x61: // ISO IR 33  - Interrupt
        case 0x62: // ISO IR 34  - Enable Manual Input
        case 0x63: // ISO IR 35  - Reset to Initial State
        case 0x64: // ISO IR 189 - Coding Method Delimiter
          result += (char16_t)(0xe000 + ch);
          ++ptr;
          break;
        case 0x6e: // ISO IR 62  - LS2
          gl = ELEMENT_G2;
          ++ptr;
          break;
        case 0x6f: // ISO IR 63  - LS3
          gl = ELEMENT_G3;
          ++ptr;
          break;
        case 0x7c: // ISO IR 64  - LS3R
          if (mode == EIGHT_BIT)
            gr = ELEMENT_G3;
          else
            gl = ELEMENT_G3;
          ++ptr;
          break;
        case 0x7d: // ISO IR 65  - LS2R
          if (mode == EIGHT_BIT)
            gr = ELEMENT_G2;
          else
            gl = ELEMENT_G2;
          ++ptr;
          break;
        case 0x7e: // ISO IR 66  - LS1R
          if (mode == EIGHT_BIT)
            gr = ELEMENT_G1;
          else
            gl = ELEMENT_G1;
          ++ptr;
          break;
        }
        continue;
      }

      if (ch >= 0x30 && ch <= 0x3f) {
        /* Type Fp - Private control function.  We map these to the PUA by
           adding 0xe000. */
        state = NORMAL;
        result += (char16_t)(0xe000 + ch);
        ++ptr;
        continue;
      }

      // ch must be in column 2 at this point
      if (ch < 0x20 || ch > 0x2f) {
        state = NORMAL;
        result += u'\u241b';
        continue;
      }

      switch (ch) {
      case 0x20: // Announce Code Structure
        state = ANNOUNCE_CODE_STRUCTURE;
        ++ptr;
        break;
      case 0x21: // C0-DESIGNATE
        state = C0_DESIGNATE;
        ++ptr;
        break;
      case 0x22: // C1-DESIGNATE
        state = C1_DESIGNATE;
        ++ptr;
        break;
      case 0x23: // Registered single control functions (see 6.5.2)
        state = SINGLE_CONTROL_FUNCTION;
        ++ptr;
        break;
      case 0x24: // Designation of multiple-byte graphic character sets (13.2.3)
        state = DESIGNATE_MULTIBYTE;
        ++ptr;
        break;
      case 0x25: // DESIGNATE OTHER CODING SYSTEM (15.4)
        state = DESIGNATE_OTHER_CODING_SYSTEM;
        ++ptr;
        break;
      case 0x26: // IDENTIFY REVISED REGISTRATION (14.5)
        state = IDENTIFY_REVISED_REGISTRATION;
        ++ptr;
        break;
      case 0x27: // Reserved
        state = NORMAL;
        result += u'\u241b';
        // *Don't* consume the character
        break;
      case 0x28: // G0-DESIGNATE 94-SET (14.3)
        state = G0_DESIGNATE_94_SET;
        ++ptr;
        break;
      case 0x29: // G1-DESIGNATE 94-SET (14.3)
        state = G1_DESIGNATE_94_SET;
        ++ptr;
        break;
      case 0x2a: // G2-DESIGNATE 94-SET (14.3)
        state = G2_DESIGNATE_94_SET;
        ++ptr;
        break;
      case 0x2b: // G3-DESIGNATE 94-SET (14.3)
        state = G3_DESIGNATE_94_SET;
        ++ptr;
        break;
      case 0x2c: // Reserved
        state = NORMAL;
        result += u'\u241b';
        // *Don't* consume the character
        break;
      case 0x2d: // G1-DESIGNATE 96-SET (14.3)
        state = G1_DESIGNATE_96_SET;
        ++ptr;
        break;
      case 0x2e: // G2-DESIGNATE 96-SET (14.3)
        state = G2_DESIGNATE_96_SET;
        ++ptr;
        break;
      case 0x2f: // G3-DESIGNATE 96-SET (14.3)
        state = G3_DESIGNATE_96_SET;
        ++ptr;
        break;
      }
      break;

    case ANNOUNCE_CODE_STRUCTURE:
      if (ch < 0x41 || ch > 0x7f) {
        state = NORMAL;
        result += u"\u241b\x20";
        // *Don't* consume this character
        continue;
      }

      {
        int facility = ch - 0x40;

        // See Table 7, 15.2.2
        switch (facility) {
        case 10:
          mode = SEVEN_BIT;     
          break;
        case 11:
          mode = EIGHT_BIT;
          break;
        case 28:
          ssarea = SINGLE_SHIFT_AREA_GR;
          break;
        default:
          // We can ignore this facility code
          break;
        }

        state = NORMAL;
        ++ptr;
      }
      break;

    case C0_DESIGNATE:
      state = NORMAL;

      if (ch < 0x40 || ch > 0x7f) {
        result += u"\u241b\x21";
        // *Don't* consume this character
        continue;
      }

      {
        unsigned csid = ctbl0[ch - 0x40];
        codeset *cset = nullptr;

        if (csid)
          cset = cf.get_codeset (csid);

        if (cset && cset->type() != C0) {
          cset->release();
          result += u"\u241b\x21";
          // *Don't* consume this character
          continue;
        }

        if (c[0])
          c[0]->release();
        c[0] = (control_codeset *)cset;
        ++ptr;
      }
      break;

    case C1_DESIGNATE:
      state = NORMAL;

      if (ch < 0x40 || ch > 0x7f) {
        result += u"\u241b\x22";
        // *Don't* consume this character
        continue;
      }

      {
        unsigned csid = ctbl1[ch - 0x40];
        codeset *cset = nullptr;

        if (csid)
          cset = cf.get_codeset (csid);

        if (cset && cset->type() != C1) {
          result += u"\u241b\x22";
          // *Don't* consume this character
          continue;
        }

        if (c[1])
          c[1]->release();
        c[1] = (control_codeset *)cset;
        ++ptr;
      }
      break;

    case SINGLE_CONTROL_FUNCTION:
      state = NORMAL;

      result += u"\u241b\x23";
      // *Don't* consume this character
      break;

    case DESIGNATE_MULTIBYTE:
      switch (ch) {
        /* 4/0, 4/1 and 4/2 were registered before the intermediate octet was
           added to the standard---see Note 46, below Table 6 */
      case 0x40:
      case 0x41:
      case 0x42:
        state = G0_DESIGNATE_MULTIBYTE_94_SET_OLD;
        // *Don't* consume this character
        continue;
      case 0x28: state = G0_DESIGNATE_MULTIBYTE_94_SET; break;
      case 0x29: state = G1_DESIGNATE_MULTIBYTE_94_SET; break;
      case 0x2a: state = G2_DESIGNATE_MULTIBYTE_94_SET; break;
      case 0x2b: state = G3_DESIGNATE_MULTIBYTE_94_SET; break;
        /* 0x2c IS NOT MISSING. It's reserved.  MULE Emacs might generate it,
           but it shouldn't be in a proper ISO 2022 encoded string. */
      case 0x2d: state = G1_DESIGNATE_MULTIBYTE_96_SET; break;
      case 0x2e: state = G2_DESIGNATE_MULTIBYTE_96_SET; break;
      case 0x2f: state = G3_DESIGNATE_MULTIBYTE_96_SET; break;
      default:
        state = NORMAL;
        result += u"\u241b\x24";
        // *Don't* consume this character
        continue;
      }

      ++ptr;
      break;

    case G0_DESIGNATE_MULTIBYTE_94_SET_OLD:
      state = NORMAL;

      // Should be impossible, but belt & braces and all that :-)
      if (ch < 0x40 || ch > 0x42) {
        result += u"\u241b\x24";
        // *Don't* consume this character
        continue;
      }

      {
        unsigned csid = gtbl4[ch - 0x40];
        codeset *cset = nullptr;

        if (csid)
          cset = cf.get_codeset (csid);

        if (cset && cset->type() != M) {
          result += u"\u241b\x24";
          // *Don't* consume this character
          continue;
        }

        if (g[0])
          g[0]->release();
        g[0] = (graphic_codeset *)cset;
        ++ptr;
      }
      break;
    
    case G0_DESIGNATE_MULTIBYTE_94_SET:
      state = NORMAL;

      if (ch == 0x20) {
        drcs_count = 0;
        drcs_id = 0;
        state = G0_DESIGNATE_MULTIBYTE_94_SET_DRCS;
        ++ptr;
        break;
      }

      if (ch < 0x43 || ch > 0x7f) {
        result += u"\u241b\x24\x28";
        // *Don't* consume this character
        continue;
      }

      // Fall through

    case G1_DESIGNATE_MULTIBYTE_94_SET:
    case G2_DESIGNATE_MULTIBYTE_94_SET:
    case G3_DESIGNATE_MULTIBYTE_94_SET:
      if (ch == 0x20) {
        drcs_count = 0;
        drcs_id = 0;
        switch (state) {
        case G1_DESIGNATE_MULTIBYTE_94_SET:
          state = G1_DESIGNATE_MULTIBYTE_94_SET_DRCS;
          break;
        case G2_DESIGNATE_MULTIBYTE_94_SET:
          state = G2_DESIGNATE_MULTIBYTE_94_SET_DRCS;
          break;
        case G3_DESIGNATE_MULTIBYTE_94_SET:
          state = G3_DESIGNATE_MULTIBYTE_94_SET_DRCS;
          break;
        default:
          // Not possible
          break;
        }
        ++ptr;
        break;
      }

      {
        int set = state - G0_DESIGNATE_MULTIBYTE_94_SET;

        state = NORMAL;

        if (ch < 0x40 || ch > 0x7f) {
          result += u"\u241b\x24";
          result += u'\x28' + set;
          // *Don't* consume this character
          continue;
        }

        unsigned csid = gtbl4[ch - 0x40];
        codeset *cset = nullptr;

        if (csid)
          cf.get_codeset (csid);

        if (cset && cset->type() != M) {
          cset->release();
          result += u"\u241b\x24";
          result += u'\x28' + set;
          // *Don't* consume this character
          continue;
        }

        if (g[set])
          g[set]->release();
        g[set] = (graphic_codeset *)cset;
        ++ptr;
      }
      break;

    case G1_DESIGNATE_MULTIBYTE_96_SET:
    case G2_DESIGNATE_MULTIBYTE_96_SET:
    case G3_DESIGNATE_MULTIBYTE_96_SET:
      state = NORMAL;

      if (ch == 0x20) {
        drcs_count = 0;
        drcs_id = 0;
        switch (state) {
        case G1_DESIGNATE_MULTIBYTE_96_SET:
          state = G1_DESIGNATE_MULTIBYTE_96_SET_DRCS;
          break;
        case G2_DESIGNATE_MULTIBYTE_96_SET:
          state = G2_DESIGNATE_MULTIBYTE_96_SET_DRCS;
          break;
        case G3_DESIGNATE_MULTIBYTE_96_SET:
          state = G3_DESIGNATE_MULTIBYTE_96_SET_DRCS;
          break;
        default:
          // Not possible
          break;
        }
        ++ptr;
        break;
      }

      if (ch < 0x40 || ch > 0x7f) {
        result += u"\u241b\x24";
        result += u'\x2c' + 1 + state - G1_DESIGNATE_MULTIBYTE_94_SET;
        // *Don't* consume this character
        continue;
      }

      {
        int set = 1 + state - G1_DESIGNATE_MULTIBYTE_94_SET;
        unsigned csid = gtbl4[ch - 0x40];
        codeset *cset = nullptr;

        if (csid)
          cset = cf.get_codeset (csid);

        if (cset && cset->type() != M) {
          cset->release();
          result += u"\u241b\x24";
          result += u'\x2c' + set;
          // *Don't* consume this character
          continue;
        }

        if (g[set])
          g[set]->release();
        g[set] = (graphic_codeset *)cset;
        ++ptr;
      }
      break;
      
    case DESIGNATE_OTHER_CODING_SYSTEM:

      if (ch >= 0x20 && ch <= 0x2f) {
        // 15.4.2
        //
        // DOCS with I byte 2/15 shall mean that the other coding system does
        // not use DOCS (F = 04/00) coded as specified here, to return...
        //
        // DOCS with any other I byte or with no I byte, shall mean that the
        // other coding system uses DOCS (F = 04/00) to return.
        if (ch == 0x2f)
          state = DESIGNATE_OTHER_CODING_SYSTEM_WO_SR;
        else
          state = DESIGNATE_OTHER_CODING_SYSTEM_W_SR;
        docs_flag = ch;
        ++ptr;
        continue;
      }

      docs_flag = 0;

      // Fall through

    case DESIGNATE_OTHER_CODING_SYSTEM_W_SR:
      state = NORMAL;
      if (ch < 0x40 || ch > 0x7f) {
        result += u"\u241b\x25";
        if (docs_flag)
          result += (char16_t)docs_flag;

        // *Don't* consume this character
        continue;
      }

      {
        unsigned csid = docs1[ch - 0x40];
        codeset *cset = nullptr;

        if (csid)
          cset = cf.get_codeset (csid);

        // If we don't know what codeset to use, generate U+FFFD
        if (!cset) {
          enum { N, E, E25 } sr_state = N;
          bool exit = false;

          while (!exit && ++ptr < end) {
            switch (sr_state) {
            case N:
              if (*ptr == ESC)
                sr_state = E;
              else
                result += u'\ufffd';
              break;
            case E:
              if (*ptr == 0x25)
                sr_state = E25;
              else {
                sr_state = N;
                result += u'\ufffd';
                continue;
              }
              break;
            case E25:
              sr_state = N;
              if (*ptr == 0x40)
                exit = true;
              else {
                result += u"\ufffd\ufffd";
                continue;
              }
              break;
            }
          }

          switch (sr_state) {
          case N:
            break;
          case E:
            result += u'\ufffd';
            break;
          case E25:
            result += u"\ufffd\ufffd";
            break;
          }

          continue;
        }

        if (cset->type() != wSR) {
          result += u"\u241b\x25";
          if (docs_flag)
            result += (char16_t)docs_flag;

          // *Don't* consume this character
          continue;
        }

        ++ptr;

        docs_codeset *dcs = (docs_codeset *)cset;

        dcs->decode (ptr, end, result);

        dcs->release();
      }
      break;

    case DESIGNATE_OTHER_CODING_SYSTEM_WO_SR:
      state = NORMAL;
      if (ch < 0x40 || ch > 0x7f) {
        result += u"\u241b\x25\x2f";
        // *Don't* consume this character
        continue;
      }

      {
        unsigned csid = docs1[ch - 0x40];
        codeset *cset = nullptr;

        if (csid)
          cset = cf.get_codeset (csid);
       
        if (!cset) {
          while (++ptr < end)
            result += u'\ufffd';
          break;
        }

        if (cset->type() != woSR) {
          if (cset)
            cset->release();
          result += u"\u241b\x25\x2f";
          // *Don't* consume this character
          continue;
        }

        ++ptr;

        docs_codeset *dcs = (docs_codeset *)cset;

        dcs->decode (ptr, end, result);

        dcs->release(); 
      }
      break;

    case IDENTIFY_REVISED_REGISTRATION:
      state = NORMAL;
      if (ch < 0x40 || ch > 0x7f) {
        result += u"\u241b\x26";
        // *Don't* consume this character
        continue;
      }

      // We ignore IDENTIFY REVISED REGISTRATION
      ++ptr;
      break;

    case G0_DESIGNATE_94_SET:
    case G1_DESIGNATE_94_SET:
    case G2_DESIGNATE_94_SET:
    case G3_DESIGNATE_94_SET:
      if (ch == 0x20) {
        drcs_count = 0;
        drcs_id = 0;
        switch (state) {
        case G0_DESIGNATE_94_SET: state = G0_DESIGNATE_94_SET_DRCS; break;
        case G1_DESIGNATE_94_SET: state = G1_DESIGNATE_94_SET_DRCS; break;
        case G2_DESIGNATE_94_SET: state = G2_DESIGNATE_94_SET_DRCS; break;
        case G3_DESIGNATE_94_SET: state = G3_DESIGNATE_94_SET_DRCS; break;
        default:
          // Cannot happen
          break;
        }
        ++ptr;
        break;
      }

      if (ch == 0x21) {
        switch (state) {
        case G0_DESIGNATE_94_SET: state = G0_DESIGNATE_94_SET_2; break;
        case G1_DESIGNATE_94_SET: state = G1_DESIGNATE_94_SET_2; break;
        case G2_DESIGNATE_94_SET: state = G2_DESIGNATE_94_SET_2; break;
        case G3_DESIGNATE_94_SET: state = G3_DESIGNATE_94_SET_2; break;
        default:
          // Cannot happen
          break;
        }
        ++ptr;
        break;
      }

      {
        int set = state - G0_DESIGNATE_94_SET;
        
        state = NORMAL;

        if (ch < 0x40 || ch > 0x7f) {
          result += u"\u241b";
          result += u'\x28' + set;
          // *Don't* consume this character
          continue;
        }

        unsigned csid = gtbl1[ch - 0x40];
        codeset *cset = nullptr;

        if (csid)
          cset = cf.get_codeset (csid);

        if (cset && cset->type() != G94) {
          if (cset)
            cset->release();
          result += u"\u241b";
          result += u'\x28' + set;
          // *Don't* consume this character
          continue;
        }

        if (g[set])
          g[set]->release();
        g[set] = (graphic_codeset *)cset;
        ++ptr;
      }
      break;

    case G0_DESIGNATE_94_SET_2:
    case G1_DESIGNATE_94_SET_2:
    case G2_DESIGNATE_94_SET_2:
    case G3_DESIGNATE_94_SET_2:
      {
        int set = state - G0_DESIGNATE_94_SET_2;
        state = NORMAL;

        if (ch < 0x40 || ch > 0x7f) {
          result += u"\u241b";
          result += u'\x28' + set;
          result += u'\x21';
          // *Don't* consume this character
          continue;
        }

        unsigned csid = gtbl2[ch - 0x40];
        codeset *cset = nullptr;

        if (csid)
          cset = cf.get_codeset (csid);

        if (cset && cset->type() != G94) {
          cset->release();
          result += u"\u241b";
          result += u'\x28' + set;
          result += u'\x21';
          // *Don't* consume this character
          continue;
        }

        if (g[set])
          g[set]->release();
        g[set] = (graphic_codeset *)cset;
        ++ptr;
      }
      break;

    case G1_DESIGNATE_96_SET:
    case G2_DESIGNATE_96_SET:
    case G3_DESIGNATE_96_SET:
      if (ch == 0x20) {
        drcs_count = 0;
        drcs_id = 0;
        switch (state) {
        case G1_DESIGNATE_96_SET: state = G1_DESIGNATE_96_SET_DRCS; break;
        case G2_DESIGNATE_96_SET: state = G2_DESIGNATE_96_SET_DRCS; break;
        case G3_DESIGNATE_96_SET: state = G3_DESIGNATE_96_SET_DRCS; break;
        default:
          // Cannot happen
          break;
        }
        ++ptr;
        break;
      }

      {
        int set = 1 + state - G1_DESIGNATE_96_SET;
        state = NORMAL;

        if (ch < 0x40 || ch > 0x7f) {
          result += u"\u241b";
          result += u'\x2c' + set;

          // *Don't* consume this character
          continue;
        }

        unsigned csid = gtbl3[ch - 0x40];
        codeset *cset = nullptr;

        if (csid)
          cset = cf.get_codeset (csid);

        if (cset && cset->type() != G96) {
          cset->release();
          result += u"\u241b";
          result += u'\x2c' + set;
          // *Don't* consume this character
          continue;
        }

        if (g[set])
          g[set]->release();
        g[set] = (graphic_codeset *)cset;
        ++ptr;
      }
      break;

    case G0_DESIGNATE_MULTIBYTE_94_SET_DRCS:
    case G1_DESIGNATE_MULTIBYTE_94_SET_DRCS:
    case G2_DESIGNATE_MULTIBYTE_94_SET_DRCS:
    case G3_DESIGNATE_MULTIBYTE_94_SET_DRCS:
      if (ch >= 0x20 && ch <= 0x2f) {
        if (drcs_count >= 7) {
          int set = state - G0_DESIGNATE_MULTIBYTE_94_SET_DRCS;

          state = NORMAL;

          result += u"\u241b\x24";
          result += u'\x28' + set;
          result += u'\x20';
          while (drcs_count--)
            result += u'\x20' + ((drcs_id >> (4 * drcs_count)) & 0x0f);
          // *Don't* consume this character
          continue;
        }

        ++drcs_count;
        drcs_id = (drcs_id << 4) | (ch & 0x0f);
        ++ptr;
        continue;
      }

      {
        int set = state - G0_DESIGNATE_MULTIBYTE_94_SET_DRCS;

        state = NORMAL;

        if (ch < 0x40 || ch > 0x7f) {
          result += u"\u241b\x24";
          result += u'\x28' + set;
          result += u'\x20';
          while (drcs_count--)
            result += u'\x20' + ((drcs_id >> (4 * drcs_count)) & 0x0f);
          // *Don't* consume this character
          continue;
        }

        drcs_id |= CODESET_DRCS;

        codeset *cset = cf.get_codeset (drcs_id);

        if (cset && cset->type() != M) {
          cset->release();
          result += u"\u241b\x24";
          result += u'\x28' + set;
          result += u'\x20';
          while (drcs_count--)
            result += u'\x20' + ((drcs_id >> (4 * drcs_count)) & 0x0f);
          // *Don't* consume this character
          continue;
        }

        if (g[set])
          g[set]->release();
        g[set] = (graphic_codeset *)cset;
        ++ptr;
      }
      break;

    case G1_DESIGNATE_MULTIBYTE_96_SET_DRCS:
    case G2_DESIGNATE_MULTIBYTE_96_SET_DRCS:
    case G3_DESIGNATE_MULTIBYTE_96_SET_DRCS:
      if (ch >= 0x20 && ch <= 0x2f) {
        if (drcs_count >= 7) {
          int set = state + 1 - G1_DESIGNATE_MULTIBYTE_96_SET_DRCS;

          state = NORMAL;

          result += u"\u241b\x24";
          result += u'\x2c' + set;
          result += u'\x20';
          while (drcs_count--)
            result += u'\x20' + ((drcs_id >> (4 * drcs_count)) & 0x0f);
          // *Don't* consume this character
          continue;
        }

        ++drcs_count;
        drcs_id = (drcs_id << 4) | (ch & 0x0f);
        ++ptr;
        continue;
      }

      {
        int set = state + 1 - G1_DESIGNATE_MULTIBYTE_96_SET_DRCS;

        state = NORMAL;

        if (ch < 0x40 || ch > 0x7f) {
          result += u"\u241b\x24";
          result += u'\x2c' + set;
          result += u'\x20';
          while (drcs_count--)
            result += u'\x20' + ((drcs_id >> (4 * drcs_count)) & 0x0f);
          // *Don't* consume this character
          continue;
        }

        drcs_id |= CODESET_DRCS;

        codeset *cset = cf.get_codeset (drcs_id);

        if (cset && cset->type() != M) {
          cset->release();
          result += u"\u241b\x24";
          result += u'\x2c' + set;
          result += u'\x20';
          while (drcs_count--)
            result += u'\x20' + ((drcs_id >> (4 * drcs_count)) & 0x0f);
          // *Don't* consume this character
          continue;
        }

        if (g[set])
          g[set]->release();
        g[set] = (graphic_codeset *)cset;
        ++ptr;
      }
      break;

    case G0_DESIGNATE_94_SET_DRCS:
    case G1_DESIGNATE_94_SET_DRCS:
    case G2_DESIGNATE_94_SET_DRCS:
    case G3_DESIGNATE_94_SET_DRCS:
      if (ch >= 0x20 && ch <= 0x2f) {
        if (drcs_count >= 7) {
          int set = state - G0_DESIGNATE_94_SET_DRCS;

          state = NORMAL;

          result += u'\u241b';
          result += u'\x28' + set;
          result += u'\x20';
          while (drcs_count--)
            result += u'\x20' + ((drcs_id >> (4 * drcs_count)) & 0x0f);
          // *Don't* consume this character
          continue;
        }

        ++drcs_count;
        drcs_id = (drcs_id << 4) | (ch & 0x0f);
        ++ptr;
        continue;
      }

      {
        int set = state - G0_DESIGNATE_94_SET_DRCS;

        state = NORMAL;

        if (ch < 0x40 || ch > 0x7f) {
          result += u'\u241b';
          result += u'\x28' + set;
          result += u'\x20';
          while (drcs_count--)
            result += u'\x20' + ((drcs_id >> (4 * drcs_count)) & 0x0f);
          // *Don't* consume this character
          continue;
        }

        drcs_id |= CODESET_DRCS;

        codeset *cset = cf.get_codeset (drcs_id);

        if (cset && cset->type() != G94) {
          cset->release();
          result += u'\u241b';
          result += u'\x28' + set;
          result += u'\x20';
          while (drcs_count--)
            result += u'\x20' + ((drcs_id >> (4 * drcs_count)) & 0x0f);
          // *Don't* consume this character
          continue;
        }

        if (g[set])
          g[set]->release();
        g[set] = (graphic_codeset *)cset;
        ++ptr;
      }
      break;

    case G1_DESIGNATE_96_SET_DRCS:
    case G2_DESIGNATE_96_SET_DRCS:
    case G3_DESIGNATE_96_SET_DRCS:
      if (ch >= 0x20 && ch <= 0x2f) {
        if (drcs_count >= 7) {
          int set = state + 1 - G1_DESIGNATE_96_SET_DRCS;

          state = NORMAL;

          result += u'\u241b';
          result += u'\x2c' + set;
          result += u'\x20';
          while (drcs_count--)
            result += u'\x20' + ((drcs_id >> (4 * drcs_count)) & 0x0f);
          // *Don't* consume this character
          continue;
        }

        ++drcs_count;
        drcs_id = (drcs_id << 4) | (ch & 0x0f);
        ++ptr;
        continue;
      }

      {
        int set = state + 1 - G1_DESIGNATE_96_SET_DRCS;

        state = NORMAL;

        if (ch < 0x40 || ch > 0x7f) {
          result += u'\u241b';
          result += u'\x2c' + set;
          result += u'\x20';
          while (drcs_count--)
            result += u'\x20' + ((drcs_id >> (4 * drcs_count)) & 0x0f);
          // *Don't* consume this character
          continue;
        }

        drcs_id |= CODESET_DRCS;

        codeset *cset = cf.get_codeset (drcs_id);

        if (cset && cset->type() != G96) {
          cset->release();
          result += u'\u241b';
          result += u'\x2c' + set;
          result += u'\x20';
          while (drcs_count--)
            result += u'\x20' + ((drcs_id >> (4 * drcs_count)) & 0x0f);
          // *Don't* consume this character
          continue;
        }

        if (g[set])
          g[set]->release();
        g[set] = (graphic_codeset *)cset;
        ++ptr;
      }
      break;  
    }
  }

  if (g[gl])
    g[gl]->finish (result);
  if (g[gr])
    g[gr]->finish (result);

  return result;
}
