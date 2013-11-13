/* Emacs, this is -*-C++-*- */

#ifndef ISO2022_BASE_H_
#define ISO2022_BASE_H_

#define BEGIN_ISO2022_NS namespace iso2022 {
#define END_ISO2022_NS   }

BEGIN_ISO2022_NS

namespace codesets {
  enum {
    standard_c0 = 1,
    standard_c1 = 77,
    ascii       = 6,

    /* Strictly speaking each of these is only the G1 piece; generally you
       need to combine 1, 77, 6 and one of these to get the ISO set named */
    iso_8859_1  = 100,
    iso_8859_2  = 101,
    iso_8859_3  = 109,
    iso_8859_4  = 110,
    iso_8859_5  = 111,
    iso_8859_6  = 127,
    iso_8859_7  = 126,
    iso_8859_8  = 198,
    iso_8859_9  = 148,
    iso_8859_10 = 157,
    iso_8859_11 = 166,
    // iso_8859_12 doesn't exist (it was going to be Latin/Devanagari)
    iso_8859_13 = 179,
    iso_8859_14 = 199,
    iso_8859_15 = 203,
    iso_8859_16 = 226,

    // Unicode (via DOCS)
    utf_8       = 196, // unspecified level, with return via ESC 2/5 4/0
    utf_8_l1    = 190, // level 1, no return
    utf_8_l2    = 191, // level 2, no return
    utf_8_l3    = 192, // level 3, no return

    utf_16_l1   = 193, // level 1, no return
    utf_16_l2   = 194, // level 2, no return
    utf_16_l3   = 195, // level 3, no return

    ucs_2_l1    = 162, // level 1, no return
    ucs_2_l2    = 174, // level 2, no return
    ucs_2_l3    = 176, // level 3, no return

    ucs_4_l1    = 163, // level 1, no return
    ucs_4_l2    = 175, // level 2, no return
    ucs_4_l3    = 177, // level 3, no return

    // Aliases
    latin_1        = iso_8859_1,
    latin_2        = iso_8859_2,
    latin_3        = iso_8859_3,
    latin_4        = iso_8859_4,
    latin_cyrillic = iso_8859_5,
    latin_arabic   = iso_8859_6,
    latin_greek    = iso_8859_7,
    latin_hebrew   = iso_8859_8,
    latin_5        = iso_8859_9,
    latin_6        = iso_8859_10,
    latin_thai     = iso_8859_11,
    latin_7        = iso_8859_13,
    latin_8        = iso_8859_14,
    latin_9        = iso_8859_15,
    latin_10       = iso_8859_16,
  };
};

typedef enum {
  ELEMENT_G0 = 0,
  ELEMENT_G1 = 1,
  ELEMENT_G2 = 2,
  ELEMENT_G3 = 3
} code_element;

typedef enum {
  SINGLE_SHIFT_AREA_DEFAULT = 0,
  SINGLE_SHIFT_AREA_GL = 1,
  SINGLE_SHIFT_AREA_GR = 2
} single_shift_area;

typedef enum {
  EIGHT_BIT = 8,
  SEVEN_BIT = 7
} bits;

/* If ALLOW_SHIFT_CHARS is set, we allow LS0, LS1, SS2 and SS3 even if
   ALLOW_CONTROL_CHARS is not set.  Note that, in that case, we won't be
   mapping control chars through the selected C0/C1 sets.

   Also note that ALLOW_CONTROL_CHARS does not affect escape sequences.
   They are controlled, separately, by ALLOW_ESCAPES. */
enum {
  ALLOW_ESCAPES       = 0x0001,
  ALLOW_CONTROL_CHARS = 0x0002,
  ALLOW_SHIFT_CHARS   = 0x0004,
  CANONICAL_MODE      = 0x0008, // Means to use CER/DER rules about Gn use
};

/* Most of the following are not defined by ISO-2022, but are standard
   control codes that can be mapped to/from Unicode without compatibility
   problems.

   Those marked (F) have their encoding fixed by ISO-2022.

   Codes marked (*) are *used* by ISO-2022, but the encoding is not fixed(!)
   so they must be mapped before we can interpret them. */
const unsigned char NUL = 0x00;  // Null
const unsigned char SOH = 0x01;  // Start of heading
const unsigned char STX = 0x02;  // Start of text
const unsigned char ETX = 0x03;  // End of text
const unsigned char EOT = 0x04;  // End of transmission
const unsigned char ENQ = 0x05;  // Enquiry
const unsigned char ACK = 0x06;  // Acknowledge
const unsigned char BEL = 0x07;  // Bell
const unsigned char BS  = 0x08;  // Backspace
const unsigned char HT  = 0x09;  // Horizontal tab
const unsigned char LF  = 0x0a;  // Line feed
const unsigned char VT  = 0x0b;  // Vertical tab
const unsigned char FF  = 0x0c;  // Form feed
const unsigned char CR  = 0x0d;  // Carriage return
const unsigned char SO  = 0x0e;  // Shift Out (*)
const unsigned char LS0 = SO;    // (aka LS0)
const unsigned char SI  = 0x0f;  // Shift In (*)
const unsigned char LS1 = SI;    // (aka LS1)
const unsigned char DLE = 0x10;  // Data Link Escape
const unsigned char DC1 = 0x11;  // Device Control 1
const unsigned char XON = DC1;   // (aka XON)
const unsigned char DC2 = 0x12;  // Device Control 2
const unsigned char DC3 = 0x13;  // Device Control 3
const unsigned char XOFF = DC3;  // (aka XOFF)
const unsigned char DC4 = 0x14;  // Device Control 4
const unsigned char NAK = 0x15;  // Negative Acknowledge
const unsigned char SYN = 0x16;  // Synchronous Idle
const unsigned char ETB = 0x17;  // End of Transmission Block
const unsigned char CAN = 0x18;  // Cancel
const unsigned char EM  = 0x19;  // End of Medium
const unsigned char SUB = 0x1a;  // Substitute
const unsigned char ESC = 0x1b;  // Escape (F)
const unsigned char FS  = 0x1c;  // File Separator
const unsigned char GS  = 0x1d;  // Group Separator
const unsigned char RS  = 0x1e;  // Record Separator
const unsigned char US  = 0x1f;  // Unit Separator

// These are in GL, not CL or CR, but they are *usually* fixed
const unsigned char SP  = 0x20;  // Space (F)

const unsigned char DEL = 0x7f;  // Delete (F)

/* The following codes are traditional C1 codes, most of which come from
   ISO 6429 (or ISO IR 77).  Of these, ISO 2022 uses only SS2 and SS3, but
   they must be mapped first because they are not fixed. 

   Codes marked (N) are not part of ISO 6429. */
const unsigned char PAD = 0x80;  // Padding unsigned character (N)
const unsigned char HOP = 0x81;  // High Octet Present (N)
const unsigned char BPH = 0x82;  // Break Permitted Here (N)
const unsigned char NBH = 0x83;  // No Break Here (N)
const unsigned char IND = 0x84;  // Index
const unsigned char NEL = 0x85;  // Next Line
const unsigned char SSA = 0x86;  // Start of Selected Area
const unsigned char ESA = 0x87;  // End of Selected Area
const unsigned char HTS = 0x88;  // Horizontal Tabulation Set
const unsigned char HTJ = 0x89;  // Horizontal Tabulation with Justification
const unsigned char VTS = 0x8a;  // Vertical Tabulation Set
const unsigned char PLD = 0x8b;  // Partial Line Down
const unsigned char PLU = 0x8c;  // Partial Line Up
const unsigned char RI  = 0x8d;  // Reverse Index
const unsigned char SS2 = 0x8e;  // Single-Shift 2 (*)
const unsigned char SS3 = 0x8f;  // Single-Shift 3 (*)
const unsigned char DCS = 0x90;  // Device Control String
const unsigned char PU1 = 0x91;  // Private Use 1
const unsigned char PU2 = 0x92;  // Private Use 2
const unsigned char STS = 0x93;  // Set Transmit State
const unsigned char CCH = 0x94;  // Cancel Unsigned Character
const unsigned char MW  = 0x95;  // Message Waiting
const unsigned char SPA = 0x96;  // Start of Protected Area
const unsigned char EPA = 0x97;  // End of Protected Area
const unsigned char SOS = 0x98;  // Start of String (N)
const unsigned char SGCI = 0x99; // Single Graphic Character Introducer (N)
const unsigned char SCI = 0x9a;  // Single Character Introducer (N)
const unsigned char CSI = 0x9b;  // Control Sequence Introducer
const unsigned char ST  = 0x9c;  // String Terminator
const unsigned char OSC = 0x9d;  // Operating System Command
const unsigned char PM  = 0x9e;  // Privacy Message
const unsigned char APC = 0x9f;  // Application Program Command

extern const unsigned default_control_set[2];
extern const unsigned default_graphic_set[4];
extern const unsigned empty_control_set[2];
extern const unsigned empty_graphic_set[4];

END_ISO2022_NS

#endif
