// also available live: https://wandbox.org/permlink/VSrFracmUy1WzowI

struct B { virtual ~B() = default; /* ... */ };
struct D0 : B { /* ... */ };
struct D1 : B { /* ... */ };
class X {
public:
   X(int, double) {}
};
void f(D0&){}
void f(D1*){}
int main() {
   const float x = 3.14159f;
   int n = static_cast<int>(x); // Ok, no warning
   X x0{ 3, 3.5 }; // Ok
   // compiles, probably warns (narrowing conversion)
   X x1(3.5,0);
   // does not compile, narrowing not allowed with braces
   // X x2{ 3.5, 0 };
   X x3{ static_cast<int>(x), 3 }; // Ok
   D0 d0;
   // illegal, no base-derived relationship with D0 and D1
   // D1* d1 = static_cast<D1*>(&d0);
   // Ok, static_cast could be omitted
   B *b = static_cast<B*>(&d0);
   // f(*b); // illegal
   f(*static_cast<D0*>(b)); // Ok
   f(static_cast<D1*>(b)); // compiles but very dangerous!
}
