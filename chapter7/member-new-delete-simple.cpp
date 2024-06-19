// also available live: https://wandbox.org/permlink/IBbLukDJJ7TfQpdN

#include <iostream>
#include <new>

class X {
   // ...
public:
   X() { std::cout << "X::X()\n"; }
   ~X() { std::cout << "X::~X()\n"; }
   void *operator new(std::size_t);
   void *operator new[](std::size_t);
   void operator delete(void*);
   void operator delete[](void*);
   // ...
};

// ...

void* X::operator new(std::size_t n) {
   std::cout << "Some X::operator new() magic\n";
   return ::operator new(n);
}
void* X::operator new[](std::size_t n) {
   std::cout << "Some X::operator new[]() magic\n";
   return ::operator new[](n);
}
void X::operator delete(void *p) {
   std::cout << "Some X::operator delete() magic\n";
   return ::operator delete(p);
}
void X::operator delete[](void *p) {
   std::cout << "Some X::operator delete[]() magic\n";
   return ::operator delete[](p);
}
int main() {
   std::cout << "p = new int{3}\n";
   int *p = new int{ 3 }; // global operator new
   std::cout << "q = new X\n";
   X *q = new X; // X::operator new
   std::cout << "delete p\n";
   delete p; // global operator delete
   std::cout << "delete q\n";
   delete q; // X::operator delete
}
