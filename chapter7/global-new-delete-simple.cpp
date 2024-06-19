// also available live: https://wandbox.org/permlink/HrlvwrUyubLoNZaY

#include <iostream>
#include <cstdlib>
#include <new>
void *operator new(std::size_t n) {
    std::cout << "operator new(" << n << ")\n";
    auto p = std::malloc(n);
    if(!p) throw std::bad_alloc{};
    return p;
}
void operator delete(void *p) noexcept {
    std::cout << "operator delete(...)\n";
    std::free(p);
}
void operator delete(void *p, std::size_t n) noexcept {
    std::cout << "operator delete(..., " << n << ")\n";
    ::operator delete (p);
}
void *operator new[](std::size_t n) {
    std::cout << "operator new[](" << n << ")\n";
    auto p = std::malloc(n);
    if(!p) throw std::bad_alloc{};
    return p;
}
void operator delete[](void *p) noexcept {
    std::cout << "operator delete[](...)\n";
    std::free(p);
}
void operator delete[](void *p, std::size_t n) noexcept {
    std::cout << "operator delete[](..., " << n << ")\n";
    ::operator delete[] (p);
}
int main() {
   auto p = new int{ 3 };
   delete p;
   p = new int[10];
   delete []p;
}
