// also available live: https://wandbox.org/permlink/YSh0wcwPVeknE2U9

#include <iostream>
class Darn {};
void f() { throw 3; }
struct Evil {
   Evil() {
      std::cout << "Evil::Evil()\n";
   }
   ~Evil() noexcept(false) {
      std::cout << "Evil::~Evil()\n";
      throw Darn {};
   }
};
void g() {
    std::cout << "A\n";
    Evil e;
    std::cout << "B\n";
    f();
    std::cout << "C\n";
}
int main() {
   try {
      g();
   } catch(int) {
      std::cerr << "catch(int)\n";
   } catch(Darn) {
      std::cerr << "darn...\n";
   }
}