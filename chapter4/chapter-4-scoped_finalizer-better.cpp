// also available live: https://wandbox.org/permlink/1wvxvv3bcKMPQIEU

#include <iostream>
#include <vector>

template <class F> class scoped_finalizer {
   F f;
public:
   scoped_finalizer(const scoped_finalizer&) = delete;
   scoped_finalizer& operator=
      (const scoped_finalizer&) = delete;
   scoped_finalizer(F f) : f{ f } {
   }
   ~scoped_finalizer() {
      f();
   }
};

#include <cstdio>

FILE file_; // dummy

FILE * open(const char *) {
    std::cout << "Call to open\n";
    return &file_; // dummy implementation
}
void close(FILE *) {
    std::cout << "Call to close\n";    
}

int read(FILE *, char *, int ) {
    return 0; // dummy implementation
}

void process(const std::vector<char> &) {
}

enum { N = 2048 };

void f(const char *name) {
   FILE *file = open(name);
   if(!file) return; // failure
   auto sf = scoped_finalizer{ [&file] {
      close(file);
   } }; // <-- sf manages file now
   std::vector<char> v;
   char buf[N]; // N is a positive integral constant
   for(int n = read(file, buf, N); n != 0;
       n = read(file, buf, N))
      v.insert(end(v), buf + 0, buf + n);
   process(v); // our processing function
} // implicit close(file) through sf's destructor

int main() {
    f("test.txt");
}
