#include <asn1/asn1.h>
#include <iostream>
#include <iomanip>

int main (void)
{
  asn1::OID oid = { 2, 100, 3 };
  asn1::DEREncoder d(asn1::secure_allocator);
  asn1::GraphicString gs = u"¡Hõla! Hasta Mañana!";
  asn1::choice<asn1::int32, asn1::OID, bool> c;
  std::vector<asn1::octet> bytes = { 0x01, 0x02, 0x03, 0x04 };

  std::cout << oid << std::endl;

  c = oid;
  c = true;

  d << asn1::sequence
    << asn1::set << 5 << -7l << 0.0625 << bytes << asn1::end
    << oid
    << asn1::instance_of(oid) << 548 << asn1::end
    << asn1::enumerated(123)
    << gs
    << c
    << asn1::end;

  std::cout << d.asDER();

  return 0;
}
