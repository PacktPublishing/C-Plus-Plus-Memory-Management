// also available live: https://wandbox.org/permlink/QFsK1GABz9iO39Ih

struct B0 {
   virtual int f() const = 0;
   virtual ~B0() = default;
};
struct B1 {
   virtual int g() const = 0;
   virtual ~B1() = default;
};
class D0 : public B0 {
   public: int f() const override { return 3; }
};
class D1 : public B1 {
   public: int g() const override { return 4; }
};
class D : public D0, public D1 {};
int f(D *p) {
   return p? p->f() + p->g() : -1; // Ok
}
// g has the wrong interface: it accepts a D0& but
// tries to use it as a D1&, which makes sense if
// the referred object is publicly D0 and D1 (for
// example, class D
int g(D0 &d0) {
   D1 &d1 = dynamic_cast<D1&>(d0); // throws if wrong
   return d1.g();
}
int main() {
   D d;
   f(&d); // Ok
   g(d); // Ok, a D is a D0
   D0 d0;
   // calls f(nullptr) as &d0 does not point to a D
   f(dynamic_cast<D*>(&d0));
   g(d0); // compiles but will throw bad_cast
}
