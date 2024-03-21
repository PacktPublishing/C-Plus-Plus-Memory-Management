// also available live: https://wandbox.org/permlink/ZOENIi17vqNEt06l

#include <cstring> // std::strlen()
#include <algorithm> // std::copy()
#include <utility> // std::swap()
class naive_string { // too simple to be useful
   char *p {}; // pointer to the elements (nullptr)
   std::size_t nelems {}; // number of elements (zero)
public:
   std::size_t size() const {
      return nelems;
   }
   bool empty() const {
      return size() == 0;
   }
   naive_string() = default; // empty string
   naive_string(const char *s)
      : nelems{ std::strlen(s) } {
      p = new char[size() + 1]; // leaving room for a 
                                // (convenient) trailing 0
      std::copy(s, s + size(), p);
      p[size()] = '\0'; 
   }
   // index-wise access to characters, const and non-const
   // versions: the const version is useful for const
   // naive_string objects, whereas the non-const version
   // lets user code modify elements
   // precondition: n < size()
   char operator[](std::size_t n) const { return p[n]; }
   char& operator[](std::size_t n) { return p[n]; }
   ~naive_string() {
      delete [] p;
   }
   naive_string(const naive_string &other)
      : p{ new char[other.size() + 1] },
        nelems{ other.size() } {
      std::copy(other.p, other.p + other.size(), p);
      p[size()] = '\0';
   }
   void swap(naive_string &other) noexcept {
      using std::swap; // make the standard swap function
                       // available
      swap(p, other.p); // swap data members
      swap(nelems, other.nelems);
   }
   // idiomatic copy assignment
   naive_string& operator=(const naive_string &other) {
      naive_string { other }.swap(*this); // <-- here
      return *this; // yes, that's it!
   }
   naive_string(naive_string &&other) noexcept
      : p{ std::exchange(other.p, nullptr) },
        nelems{ std::exchange(other.nelems, 0) } {
   }
   naive_string& operator=(naive_string &&other) noexcept {
      naive_string { std::move(other) }.swap(*this);
      return *this;
   }
};


void f(naive_string) {} // pass-by-value

void copy_construction_examples() {
   naive_string s0{ "What a fine day" };
   naive_string s1 = s0; // constructs s1 so this is
                         // copy construction
   naive_string s2(s0); // ...this too
   naive_string s3{ s0 }; // ...and so is this
   f(s0); // likewise because of pass-by-value
   s1 = s0; // this is not a copy construction as s1
            // already exists: this is a copy assignment
}

int main() {
   copy_construction_examples();
}