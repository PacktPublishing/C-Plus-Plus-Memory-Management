// also available live: https://wandbox.org/permlink/mAHNOtToEmcJjGwS

#include <memory>
#include <iostream>

struct Y;
struct X {
  std::shared_ptr<Y> p;  
  ~X() { std::cout << "~X()\n"; }
};

struct Y {
  std::shared_ptr<X> p;
  ~Y() { std::cout << "~Y()\n"; }  
};

void oops() {
  auto x = std::make_shared<X>();
  auto y = std::make_shared<Y>();
  x->p = y;
  y->p = x;
}

int main() {
   oops();
   std::cout << "Done\n";
}