#include <asn1/traits.h>
#include <asn1/Tag.h>

using namespace asn1;

const Tag traits<bool>::tag = tBoolean;

const Tag traits<int32>::tag = tInteger;
const Tag traits<uint32>::tag = tInteger;
const Tag traits<int64>::tag = tInteger;
const Tag traits<uint64>::tag = tInteger;

const Tag traits<double>::tag = tReal;

const Tag traits<OID>::tag = tOID;
const Tag traits<RelativeOID>::tag = tRelativeOID;

const Tag traits<BMPString>::tag = tBMPString;
const Tag traits<UniversalString>::tag = tUniversalString;
const Tag traits<GeneralString>::tag = tGeneralString;
const Tag traits<GraphicString>::tag = tGraphicString;
const Tag traits<IA5String>::tag = tIA5String;
const Tag traits<NumericString>::tag = tNumericString;
const Tag traits<PrintableString>::tag = tPrintableString;
const Tag traits<T61String>::tag = tT61String;
const Tag traits<UTF8String>::tag = tUTF8String;
const Tag traits<VideotexString>::tag = tVideotexString;
const Tag traits<ISO646String>::tag = tISO646String;
