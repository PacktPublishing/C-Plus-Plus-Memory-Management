// also available live: https://wandbox.org/permlink/PamPZDAir10XqTnq

#include <bit>
struct A { int a; double b; };
struct B { unsigned int c; double d; };
int main() {
   // suppose this for the sake of this example
   static_assert(sizeof(int) == sizeof(float));
   constexpr A a{ 3, 3.5 }; // ok
   constexpr B b = std::bit_cast<B>(a); // Ok
   static_assert(a.a == b.c && a.b == b.d); // Ok
   static_assert((void*)&a != (void*)&b); // Ok
}
