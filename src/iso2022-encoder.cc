#include <iso2022/encoder.h>

using namespace iso2022;

const unsigned iso2022::empty_control_set[2] = { 0, 0 };
const unsigned iso2022::empty_graphic_set[4] = { 0, 0, 0, 0 };

encoder::encoder(codeset_factory &cset_factory,
                 bits m,
                 const unsigned control[2],
                 const unsigned graphic[4],
                 code_element initial_gl,
                 code_element initial_gr,
                 single_shift_area ssa,
                 unsigned flgs)
  : cf(cset_factory), flags(flgs)
{
  mode = m;

  for (unsigned n = 0; n < 2; ++n)
    ic[n] = c[n] = nullptr;
  for (unsigned n = 0; n < 4; ++n) {
    ig[n] = g[n] = nullptr;
    last_used[n] = 0;
  }
  clock = 0;

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

  gl = initial_gl;
  gr = initial_gr;

  if (ssa == SINGLE_SHIFT_AREA_DEFAULT) {
    if (mode == EIGHT_BIT)
      ssa = SINGLE_SHIFT_AREA_GR;
    else
      ssa = SINGLE_SHIFT_AREA_GL;
  }

  ssarea = ssa;
}

encoder::~encoder()
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
encoder::reset()
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
  for (auto i = graphic_sets.begin(); i < graphic_sets.end(); ++i)
    (*i)->release();
  graphic_sets.clear();
}

void
encoder::set_permitted_graphic_codesets (const std::vector<unsigned> &codesets)
{
  set_permitted_graphic_codesets (codesets.data(), codesets.size());
}

void
encoder::set_permitted_graphic_codesets (const unsigned *codesets,
                                         unsigned count)
{
  graphic_sets.clear();

  for (unsigned n = 0; n < count; ++n) {
    codeset *cset = cf.get_codeset (codesets[n]);

    if (!cset) {
      char buffer[80];
      std::sprintf (buffer, "Unknown graphic code set ISO IR %d", 
                    codesets[n]);
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
                      "Attempted to select control codeset ISO IR %d as a"
                      " permitted graphic set",
                      codesets[n]);
        cset->release();
        throw std::runtime_error(buffer);
      }
      break;
    case wSR:
    case woSR:
      {
        char buffer[80];
        std::sprintf (buffer,
                      "Attempted to select non-ISO 2022 codeset ISO IR %d"
                      " as a permitted graphic set",
                      codesets[n]);
        cset->release();
        throw std::runtime_error(buffer);
      }
    }

    graphic_sets.push_back ((graphic_codeset *)cset);
  }
}

bool
encoder::is_combining (char16_t ch)
{
  // This is perhaps a little simplistic, but it should function acceptably
  return ((ch >= 0x0300 && ch <= 0x036f)
          || (ch >= 0x1dc0 && ch <= 0x1dff)
          || (ch >= 0x20d0 && ch <= 0x20ff));
}

std::string
encoder::encode (const char16_t *str, size_t len, char replacement)
{
  const char16_t *ptr = str;
  const char16_t *end = ptr + len;
  std::string result;

  while (ptr < end) {
    auto pcs = ptr;
    char16_t ch = *pcs;
    code_element replace_elt = ELEMENT_G0, replace_elt_ng0 = ELEMENT_G1;

    // Read one Unicode character
    if (ch >= 0xd800 && ch < 0xdbff) {
      if (ptr >= end) {
        result += replacement;
        break;
      }

      if (*ptr < 0xdc00 || *ptr > 0xdfff) {
        ++pcs;
        result += replacement;
        goto done;
      }

      char32_t ch32 = 0x10000 + (((ch & 0x3ff) << 10) | (*ptr & 0x3ff));

      if (ch32 > 0x10ffff) {
        pcs += 2;
        result += replacement;
        goto done;
      }

      pcs += 2;
    } else if (ch >= 0xdc00 && ch <= 0xdfff) {
      ++pcs;
      result += replacement;
      goto done;
    } else {
      ++pcs;
    }

    // If it's a control character, try the relevant Cn
    if (ch < 0x20) {
      int mch = -1;
      if (c[0])
        mch = c[0]->encode (ch);
      if (mch < 0)
        mch = replacement;
      result += (char)mch;
      goto done;
    } else if (ch >= 0x80 && ch <= 0xa0) {
      int mch = -1;
      if (c[1])
        mch = c[1]->encode (ch);
      if (mch < 0)
        mch = replacement;
      result += (char)mch;
      goto done;
    }

    // Now read over combining marks
    while (pcs < end && encoder::is_combining (ch))
      ch = *++pcs;

    // Try GL first, then GR if it exists
    if (g[gl] && g[gl]->encode (ptr, pcs, 0, result)) {
      if (!(flags & CANONICAL_MODE))
        last_used[gl] = clock++;
      goto done;
    }

    if (mode == EIGHT_BIT) {
      /* 8-bit case first */
      if (g[gr] && g[gr]->encode (ptr, pcs, 0x80, result)) {
        if (!(flags & CANONICAL_MODE))
          last_used[gr] = clock++;
        goto done;
      }

      std::string tmp;

      if (gr != 1 && g[1] && g[1]->encode (ptr, pcs, 0x80, tmp)) {
        if (!(flags & CANONICAL_MODE))
          last_used[1] = clock++;
        result += "\x1b\x7e";
        gr = ELEMENT_G1;
        result += tmp;
        goto done;
      }

      if (gr != 2 && g[2] && g[2]->encode (ptr, pcs, 0x80, tmp)) {
        if (!(flags & CANONICAL_MODE))
          last_used[2] = clock++;

        // Use SS2 if possible
        if (tmp.length() == 1) {
          // Find its encoding
          int ss2 = -1;
          if (c[1])
            ss2 = c[1]->encode (SS2);
          if (ss2 < 0 && c[0])
            ss2 = c[0]->encode (SS2);

          if (ss2 >= 0) {
            result += (char)ss2;
            if (ssarea == SINGLE_SHIFT_AREA_GR)
              result += tmp[0];
            else if (ssarea == SINGLE_SHIFT_AREA_GL)
              result += tmp[0] - 0x80;

            goto done;
          }
        }

        result += "\x1b\x7d";
        gr = ELEMENT_G2;
        result += tmp;
        goto done;
      }

      if (gr != 3 && g[3] && g[3]->encode (ptr, pcs, 0x80, tmp)) {
        if (!(flags & CANONICAL_MODE))
          last_used[3] = clock++;

        // Use SS3 if possible
        if (tmp.length() == 1) {
          // Find its encoding
          int ss3 = -1;
          if (c[1])
            ss3 = c[1]->encode (SS3);
          if (ss3 < 0 && c[0])
            ss3 = c[0]->encode (SS3);

          if (ss3 >= 0) {
            result += (char)ss3;
            if (ssarea == SINGLE_SHIFT_AREA_GR)
              result += tmp[0];
            else if (ssarea == SINGLE_SHIFT_AREA_GL)
              result += tmp[0] - 0x80;
          }

          goto done;
        }

        result += "\x1b\x7c";
        gr = ELEMENT_G3;
        result += tmp;
        goto done;
      }
    } else {
      /* Now for 7-bit */
      std::string tmp;

      if (gl != 0 && g[0] && g[0]->encode (ptr, pcs, 0, tmp)) {
        if (!(flags & CANONICAL_MODE))
          last_used[0] = clock++;

        result += (char)LS0;
        gl = ELEMENT_G0;
        result += tmp;
        goto done;
      }

      if (gl != 1 && g[1] && g[1]->encode (ptr, pcs, 0, tmp)) {
        if (!(flags & CANONICAL_MODE))
          last_used[1] = clock++;

        result += (char)LS1;
        gl = ELEMENT_G1;
        result += tmp;
        goto done;
      }

      if (gl != 2 && g[2] && g[2]->encode (ptr, pcs, 0, tmp)) {
        if (!(flags & CANONICAL_MODE))
          last_used[2] = clock++;

        // Use SS2 if possible
        if (tmp.length() == 1) {
          // Find its encoding
          int ss2 = -1;
          if (c[1])
            ss2 = c[1]->encode (SS2);
          if (ss2 < 0 && c[0])
            ss2 = c[0]->encode (SS2);

          if (ss2 >= 0) {
            if (ss2 >= 0x80) {
              result += '\x1b';
              ss2 -= 0x40;
            }
            result += (char)ss2;
            result += tmp[0];

            goto done;
          }
        }

        result += "\x1b\x6e";
        gl = ELEMENT_G2;
        result += tmp;
        goto done;
      }

      if (gl != 3 && g[3] && g[3]->encode (ptr, pcs, 0, tmp)) {
        if (!(flags & CANONICAL_MODE))
          last_used[3] = clock++;

        // Use SS3 if possible
        if (tmp.length() == 1) {
          // Find its encoding
          int ss3 = -1;
          if (c[1])
            ss3 = c[1]->encode (SS2);
          if (ss3 < 0 && c[0])
            ss3 = c[0]->encode (SS2);

          if (ss3 >= 0) {
            if (ss3 >= 0x80) {
              result += '\x1b';
              ss3 -= 0x40;
            }
            result += (char)ss3;
            result += tmp[0];

            goto done;
          }
        }

        result += "\x1b\x6f";
        gl = ELEMENT_G3;
        result += tmp;
        goto done;
      }
    }

    // If not in canonical mode, use LRU replacement policy to switch sets
    if (!(flags & CANONICAL_MODE)) {
      unsigned max_delta, max_delta2;
      for (unsigned n = 0; n < 4; ++n) {
        unsigned delta = clock - last_used[n];
        if (n == 0 || delta > max_delta) {
          max_delta = delta;
          replace_elt = (code_element)n;
        }
        if (n >= 1 && (n == 1 || delta > max_delta2)) {
          max_delta2 = delta;
          replace_elt_ng0 = (code_element)n;
        }
      }
    }

    // Neither was sufficient to encode; start looking for another encoding
    for (auto i = graphic_sets.begin(); i < graphic_sets.end(); ++i) {
      graphic_codeset *cset = *i;
      std::string tmp;

      switch (cset->type()) {
      case G94:
        if (cset->encode (ptr, pcs, 0, tmp)) {
          cset->invoke (replace_elt, result);
          if (g[replace_elt])
            g[replace_elt]->release();
          g[replace_elt] = cset;
          if (gl != replace_elt) {
            switch (replace_elt) {
            case ELEMENT_G0: result += (char)LS0; break;
            case ELEMENT_G1: result += (char)LS1; break;
            case ELEMENT_G2: result += "\x1b\x6e"; break;
            case ELEMENT_G3: result += "\x1b\x6f"; break;
            }
            gl = replace_elt;
          }

          if (!(flags & CANONICAL_MODE))
            last_used[replace_elt] = clock++;

          cset->retain();
          result += tmp;
          goto done;
        }
        break;
      case M:
        if (cset->encode (ptr, pcs, 0, tmp)) {
          cset->invoke (replace_elt, result);
          if (g[replace_elt])
            g[replace_elt]->release();
          g[replace_elt] = cset;
          if (gl != replace_elt) {
            switch (replace_elt) {
            case ELEMENT_G0: result += (char)LS0; break;
            case ELEMENT_G1: result += (char)LS1; break;
            case ELEMENT_G2: result += "\x1b\x6e"; break;
            case ELEMENT_G3: result += "\x1b\x6f"; break;
            }
            gl = replace_elt;
          }

          if (!(flags & CANONICAL_MODE))
            last_used[replace_elt] = clock++;

          cset->retain();
          result += tmp;
          goto done;
        }
        break;
      case G96:
        if (cset->encode (ptr, pcs, mode == EIGHT_BIT ? 0x80 : 0, tmp)) {
          cset->invoke (replace_elt_ng0, result);
          if (g[replace_elt_ng0])
            g[replace_elt_ng0]->release();
          g[replace_elt_ng0] = cset;
          if (mode == SEVEN_BIT) {
            if (gl != replace_elt_ng0) {
              switch (replace_elt_ng0) {
              case ELEMENT_G0: throw std::runtime_error("Cannot happen");
              case ELEMENT_G1: result += (char)LS1; break;
              case ELEMENT_G2: result += "\x1b\x6e"; break;
              case ELEMENT_G3: result += "\x1b\x6f"; break;
              }
              gl = replace_elt_ng0;
            }
          } else {
            if (gr != replace_elt_ng0) {
              switch (replace_elt_ng0) {
              case ELEMENT_G0: throw std::runtime_error("Cannot happen");
              case ELEMENT_G1: result += "\x1b\x7e"; break;
              case ELEMENT_G2: result += "\x1b\x7d"; break;
              case ELEMENT_G3: result += "\x1b\x7c"; break;
              }
              gr = replace_elt_ng0;
            }
          }

          if (!(flags & CANONICAL_MODE))
            last_used[replace_elt_ng0] = clock++;

          cset->retain();
          result += tmp;
          goto done;
        }
        break;
      default:
        break;
      }
    }

    // If we get here, there was no available encoding for this sequence
    result += replacement;

  done:
    ptr = pcs;
  }

  return result;
}
