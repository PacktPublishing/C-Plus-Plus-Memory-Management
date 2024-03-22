// also available live: https://wandbox.org/permlink/DnyWXYHdJp52htfJ

#include <iostream>
#include <format>
struct Verbose {
   int n;
   Verbose(int n) : n{ n } {
      std::cout << std::format("Verbose({})\n", n);
   } 
   ~Verbose(){
      std::cout << std::format("~Verbose({})\n", n);
   } 
};
class X {
   static inline Verbose v0 { 0 };
   Verbose v1{ 1 };
};
Verbose v2{ 2 };
static void f() {
    static Verbose v3 { 3 };
    Verbose v4{ 4 };
}
static void g() { // note : never called
    static Verbose v5 { 5 };
}
int main() {
   Verbose v6{ 6 };
   {
      Verbose v7{ 7 };
      f();
      X x;
   }
   f();
   X x;
}