// also available live: https://wandbox.org/permlink/VmobIM2RpZuPo23g

struct B0 { int n = 3; };
struct B1 { float f = 3.5f; };
// B0 is the first base subobject of D
class D : public B0, public B1 { };
#include <iostream>
int main() {
   D d;
   // b0 and &d point to the same address
   // b1 and &d do not point to the same address
   B0 *b0 = static_cast<B0*>(&d);
   B1 *b1 = static_cast<B1*>(&d);
   int n0 = b0->n; // Ok
   float f0 = b1->f; // Ok
   // r0 and &d point to the same address
   // r1 and &d also point to the same address... oops!
   B0 *r0 = reinterpret_cast<B0*>(&d); // fragile
   // r0 and b0 point to the same location;
   // we are looking for trouble but we're
   // not really in trouble yet
   std::cout << b0 << ' ' << r0 << '\n';
   B1 *r1 = reinterpret_cast<B1*>(&d); // bad idea
   // r1 and b1 do not point to the same location;
   // we are in trouble from this point on
   std::cout << b1 << ' ' << r1 << '\n';
   int nr0 = r0->n; // Ok but fragile
   // float fr0 = r1->f; // UB
}
