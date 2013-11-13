#include <iso2022/iso2022.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <codecvt>
#include <locale>

// Assumes UTF-8 terminal
std::ostream &operator<< (std::ostream &os, const std::u16string &s) {
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> conversion;
  std::string mbs = conversion.to_bytes(s);
  return os << mbs;
}

void
dumphex (std::ostream &os, const std::string &s) {
  std::ios_base::fmtflags oldFlags = os.flags();
  std::streamsize oldPrec = os.precision();
  char oldFill = os.fill();

  os << std::hex << std::noshowbase << std::internal << std::setfill('0');

  unsigned count = 0;
  for (auto i = s.begin(); i < s.end(); ++i) {
    if (count & 0xf)
      os << " ";
    else {
      if (count)
        os << std::endl;
      os << std::setw(8) << count << "  ";
    }
    ++count;

    os << std::setw(2) << (unsigned)(unsigned char)(*i);
  }

  os << std::endl;

  os.flags(oldFlags);
  os.precision(oldPrec);
  os.fill(oldFill);  
}

int
main (void)
{
  std::string tst = "Hello! \x1b\x2d\x41\xa1H\xf5la! \x1b\x2e\x62"
    "\x1b\x22\x43\nA4 is \xa4 or \x8e\xa4\n"
    "This is normal text.";
  std::string tst2 = "Hello! \x1b\x2d\x41\x0f\x21\x0eH\x0f\x75\x0ela! "
    "\x1b\x2e\x62\x1b\x22\x43\nA4 is \x0f\x24\x0e or \x1b\x4e\x24\n"
    "This is normal text.";
  std::string tst3 = "Hello! \x1b\x28\x21\x63This is a test.\x1b\x28\x42 "
    "All of the preceding characters should be U+FFFD.";
  iso2022::codeset_factory &cf = iso2022::codeset_factory::builtin();

  iso2022::decoder decoder(cf, iso2022::EIGHT_BIT);
  iso2022::decoder decoder7(cf, iso2022::SEVEN_BIT);

  std::u16string str = decoder.decode(tst);
  std::u16string str2 = decoder7.decode(tst2);
  std::u16string strx = decoder.decode(tst3);

  dumphex(std::cout, tst);
  std::cout << str << std::endl;
  dumphex(std::cout, tst2);
  std::cout << str2 << std::endl;
  dumphex(std::cout, tst3);
  std::cout << strx << std::endl;

  iso2022::encoder encoder(cf, iso2022::EIGHT_BIT);
  iso2022::encoder encoder7(cf, iso2022::SEVEN_BIT);
  unsigned codesets[] = { 100, 203, 6 };
  encoder.set_permitted_graphic_codesets(codesets, 3);
  encoder7.set_permitted_graphic_codesets(codesets, 3);

  decoder.reset();
  decoder7.reset();

  std::string str3 = encoder.encode(str);
  std::u16string str4 = decoder.decode(str3);
  dumphex (std::cout, str3);
  std::cout << str4 << std::endl;

  std::string str5 = encoder7.encode(str);
  std::u16string str6 = decoder7.decode(str5);
  dumphex (std::cout, str5);
  std::cout << str6 << std::endl;

  iso2022::encoder encoder2(cf, iso2022::EIGHT_BIT,
                            iso2022::default_control_set,
                            iso2022::default_graphic_set,
                            iso2022::ELEMENT_G0,
                            iso2022::ELEMENT_G1,
                            iso2022::SINGLE_SHIFT_AREA_DEFAULT,
                            0);
  encoder2.set_permitted_graphic_codesets(codesets, 3);

  std::string str7 = encoder2.encode(str);
  std::u16string str8 = decoder.decode(str7);
  dumphex (std::cout, str7);
  std::cout << str8 << std::endl;

  return 0;
}
