#include <asn1/strings.h>
#include <stdexcept>
#include <iostream>

int
main (void)
{
  try {
    asn1::BMPString bmp = u"\u241bA";
    asn1::GeneralString g = u"¡Hõla!";
    asn1::GraphicString gr = u"¡Hõla!";
    asn1::IA5String ia5 = "Hello World";
    asn1::NumericString n = "12345";
    asn1::PrintableString p = "Hello World";
    asn1::T61String t61 = u"¡Hõla!";
    asn1::UniversalString u = u"¡Hõla!";
    asn1::UTF8String utf8 = u"¡Hõla!";
    asn1::VideotexString vt = u"¡Hõla!";
    asn1::VisibleString v = u"Hello World";
    asn1::ISO646String i = "Hello World";
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
