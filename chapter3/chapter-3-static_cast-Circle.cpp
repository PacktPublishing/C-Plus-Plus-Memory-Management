// also available live: https://wandbox.org/permlink/KTwp16uddJty5Txa

struct Circle {
   // ...
   // the explicit keyword is applied here to avoid
   // implicitly creating a Circle from a float if that
   // is not the programmer's intent
   explicit Circle([[maybe_unused]] float radius) {
     // ...
   }
   // ...
};
void f(Circle) { /* ... */ }
int main() {
   // f(1.0f); // does not compile (Circle constructor
               // is explicit)
   f(Circle{ 1.0f }); // fine
   f(static_cast<Circle>(1.0f)); // also fine, calls the
                                 // explicit constructor
}
