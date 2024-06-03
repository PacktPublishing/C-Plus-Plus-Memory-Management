// also available live: https://wandbox.org/permlink/mhY2nLvl37Jwzf6m

#include <bit>
struct A { int a; double b; };
struct B { unsigned int c; double d; };
int main() {
   constexpr A a{ 3, 3.5 }; // ok
   constexpr B b = std::bit_cast<B>(a); // Ok
   static_assert(a.a == b.c && a.b == b.d); // Ok
   static_assert((void*)&a != (void*)&b); // Ok
}
