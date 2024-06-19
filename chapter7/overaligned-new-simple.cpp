// also available live: https://wandbox.org/permlink/A5EjPIgUEambSeVL

#include <iostream>
#include <new>
#include <ctsdlib>
#include <type_traits>
void* operator new(std::size_t n, std::align_val_t al) {
   std::cout << "new(" << n << ", align: "
             << static_cast<std::underlying_type_t<std::align_val_t>>(al) << ")\n";
   return std::aligned_alloc(static_cast<std::size_t>(al), n);
}
struct alignas(16) Float4 { float vals[4]; };
int main() {
    auto p = new Float4;
    auto q = new(std::align_val_t{ 16 }) Float4;
    // leaks, of course, but that's besides the point
}
