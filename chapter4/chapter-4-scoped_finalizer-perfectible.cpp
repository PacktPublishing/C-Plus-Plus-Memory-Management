// also available live: https://wandbox.org/permlink/JCeKpKCpoI90C6wO

#include <iostream>
#include <vector>
template <class F> class scoped_finalizer { // simplified
   F f;
public:
   scoped_finalizer(F f) : f{ f } {
   }
   ~scoped_finalizer() {
      f();
   }
};

#include <cstdio>

FILE file_; // dummy

FILE * open_file(const char *) {
    std::cout << "Call to open\n";
    return &file_; // dummy implementation
}
void close_file(FILE *) {
    std::cout << "Call to close\n";    
}

int read_from(FILE *, char *, int ) {
    return 0; // dummy implementation
}

void process(const std::vector<char> &) {
}

enum { N = 2048 };

void f(const char *name) {
   FILE *file = open_file(name);
   if(!file) return; // failure
   auto sf = scoped_finalizer{ [&file] {
      close_file(file);
   } }; // <-- sf manages file now
   std::vector<char> v;
   char buf[N]; // N is a positive integral constant
   for(int n = read_from(file, buf, N); n != 0;
       n = read_from(file, buf, N))
      v.insert(end(v), buf + 0, buf + n);
   process(v); // our processing function
} // implicit close_file(file) through sf's destructor

int main() {
    f("test.txt");
}


