// also available live: https://wandbox.org/permlink/X1Abkbu4GhVSMINU

const int N = 3; // global constant
class X {
   int n; // note: not const
public:
   X(int n) : n{ n } {
   }
   int g() { // note: not const
      return n += N; // thus, n's state can be mutated
   }
   int f() const { // const applies to this, and
                   // transitively to its members
      // return g(); // illegal as g() is not const
      return n + 1;
   }
};
int f(const int &n) { // f() will not mutate argument n
   return X{ n }.f() + 1; // X::X(int) takes its argument
                          // by value so n remains intact
}
int main() {
   int a = 4;
   a = f(a); // a is not const in main()
}
