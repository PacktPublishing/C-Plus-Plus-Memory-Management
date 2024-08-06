// also available live: https://wandbox.org/permlink/4xFcTzdSZiU73Ym5

#include <new>
#include <vector>
#include <iostream>
struct X {
   static inline int limit = 5; // toy example, not thread-safe
   void* operator new(std::size_t n) {
      std::cout << "X::operator new() called with "
                << limit << " blocks left\n";
      while (limit <= 0) {
         if (auto hdl = std::get_new_handler(); hdl)
            hdl();
         else
            throw std::bad_alloc{};
      }
      --limit;
      return ::operator new(n);
   }
   void operator delete(void* p) {
      std::cout << "X::operator delete()\n";
      ::operator delete(p);
   }
   // same for the array versions
};
int main() {
   std::set_new_handler([]() noexcept {
      std::cout << "allocation failure, "
                   "fetching more memory\n";
      X::limit = 10;
      std::set_new_handler(nullptr); // as per default
   });
   std::vector<X*> v;
   v.reserve(100);
   try {
      for (int i = 0; i != 10; ++i)
         v.emplace_back(new X);
   } catch(...) {
      // this will never be reached with this program
      std::cerr << "out of memory\n";
   }
   for (auto p : v) delete p;
}
