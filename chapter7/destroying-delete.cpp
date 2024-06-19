// also available live: https://wandbox.org/permlink/sSo0sfN08pYdM9GV

#include <new>
#include <iostream>
class Wrapper {
public:
   enum class Kind { A, B };
private:
   struct Impl {
      virtual int f() const = 0;
   };
   struct ImplA final : Impl {
      int f() const override { return 3; }
      ~ImplA() { std::cout << "Kind A\n"; }
   };
   struct ImplB final : Impl {
      int f() const override { return 4; }
      ~ImplB() { std::cout << "Kind B\n"; }
   };
   Impl *p;
   Kind kind;
   // ...
   static Impl *create(Kind kind) {
      switch(kind) {
      using enum Kind;
      case A: return new ImplA;
      case B: return new ImplB;
      }
      throw 0;
   }
public:
   Wrapper(Kind kind)
      : p{ create(kind) }, kind{ kind } {
   }
   void operator delete(Wrapper *p, std::destroying_delete_t) {
      if(p->kind == Kind::A) {
         delete static_cast<ImplA*>(p->p);
      } else {
         delete static_cast<ImplB*>(p->p);
      }
      p->~Wrapper();
      ::operator delete(p);
   }
   int f() const { return p->f(); }
};
int main() {
   using namespace std;
   auto p = new Wrapper{ Wrapper::Kind::A };
   cout << p->f() << endl;
   delete p;
   p = new Wrapper{ Wrapper::Kind::B };
   cout << p->f() << endl;
   delete p;
}