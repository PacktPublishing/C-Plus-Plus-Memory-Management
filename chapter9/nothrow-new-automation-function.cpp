// also available live: https://wandbox.org/permlink/HQ8gShGy17BH6RNb

#include <new>
#include <cstdlib>
#include <utility>

template <class T, class ... Args>
   auto try_new(Args &&... args) {
      auto p = new (std::nothrow) T(std::forward<Args>(args)...);
      if(!p) std::abort();
      return p;
   }

struct dies_when_newed {
   void* operator new(std::size_t, std::nothrow_t) { return {}; }
};

int main() {
   auto p0 = try_new<int>(); // p0 is int*, points to an int{ 0 }
   auto p1 = try_new<int>(3); // p1 is int*, points to an int{ 3 }
   auto q = try_new<dies_when_newed>(); // calls abort()
}