// also available live: (not yet :) )

#include <concepts>
#include <type_traits>

//
// we have this cool allocation algorithm that
// fits classes X and Y, but not others like Z
//
class X { /* ... */ };
class Y { /* ... */ };
class Z { /* ... */ };

template <class C>
   concept cool_alloc_algorithm =
      std::is_same_v<C, X> || std::is_same_v<C, Y>;

template <class T> requires cool_alloc_algorithm<T>
   void* operator new(std::type_identity<T>, std::size_t n) {
      // apply the cool allocation algorithm
   }
template <class T> requires cool_alloc_algorithm<T>
  void operator delete(std::type_identity<T>, void* p) {
      // apply the cool deallocation algorithm
  }

#include <memory>  
int main() {
   auto p = std::make_unique<X>(); // uses the cool algorithm
   auto q = std::make_unique<Z>(); // uses the standard algorithm
} // uses the standard algorithm for q
  // uses the cool algorithm for p
