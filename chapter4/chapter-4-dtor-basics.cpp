// also available live: https://wandbox.org/permlink/sf43YYRKu60rx49p

#include <iostream>
struct Base {
    ~Base() { std::cout << "~Base()\n"; }
};
struct DerivedFromBase : Base {
    ~DerivedFromBase() { std::cout << "~DerivedFromBase()\n"; }
};
struct VirtualBase {
    virtual ~VirtualBase() {
       std::cout << "~VirtualBase()\n";
    }
};
struct DerivedFromVirtualBase : VirtualBase {
    ~DerivedFromVirtualBase() {
       std::cout << "~DerivedFromVirtualBase()\n";
    }
};
int main() {
   {
      Base base;
   }
   {
      DerivedFromBase derivedFromBase;
   }
   std::cout << "----\n";
   Base *pBase = new DerivedFromBase;
   delete pBase; // bad
   VirtualBase *pVirtualBase = new DerivedFromVirtualBase;
   delete pVirtualBase; // Ok
}
