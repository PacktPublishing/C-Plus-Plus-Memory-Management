// also available live: https://wandbox.org/permlink/pWqzSkAa0raJx3By

#include <new>
#include <cstdlib>

#define TRY_NEW(T,...) [&] { \
   auto p = new (std::nothrow) T(__VA_ARGS__); \
   if(!p) std::abort(); \
   return p; \
}()

struct dies_when_newed {
   void* operator new(std::size_t, std::nothrow_t) { return {}; }
};

int main() {
   auto p0 = TRY_NEW(int); // p0 is int*, points to an int{ 0 }
   auto p1 = TRY_NEW(int, 3); // p1 is int*, points to an int{ 3 }
   auto q = TRY_NEW(dies_when_newed); // calls abort()
}
