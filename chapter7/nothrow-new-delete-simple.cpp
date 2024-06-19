// also available live: https://wandbox.org/permlink/0HO29oq8jVfujy3m

#include <iostream>
#include <cstdlib>
#include <new>
// for simplicity, we delegate from the
// throwing versions to the nothrow ones
void* operator new(std::size_t n, const std::nothrow_t&) {
   return std::malloc(n);
}
void* operator new(std::size_t n) {
   auto p = operator new(n, std::nothrow);
   if (!p) throw std::bad_alloc{};
   return p;
}
void operator delete(void* p, const std::nothrow_t&) noexcept {
   std::free(p);
}
void operator delete(void* p) noexcept {
   operator delete(p, std::nothrow);
}
void operator delete(void* p, std::size_t) noexcept {
   operator delete (p, std::nothrow);
}
void* operator new[](std::size_t n, const std::nothrow_t&) {
   return std::malloc(n);
}
void* operator new[](std::size_t n) {
   auto p = operator new[](n, std::nothrow);
   if (!p) throw std::bad_alloc{};
   return p;
}
void operator delete[](void* p, const std::nothrow_t&) noexcept {
   std::free(p);
}
void operator delete[](void* p) noexcept {
   operator delete[](p, std::nothrow);
}
void operator delete[](void* p, std::size_t) noexcept {
   operator delete[](p, std::nothrow);
}
int main() {
   using std::nothrow;
   auto p = new (nothrow) int{ 3 };
   delete p;
   p = new (nothrow) int[10];
   delete[]p;
}
