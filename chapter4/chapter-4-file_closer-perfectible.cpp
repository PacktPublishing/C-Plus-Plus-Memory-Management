// also available live: https://wandbox.org/permlink/nhbpmRBrBRdxBO0M

#include <iostream>
#include <vector>
#include <cstdio>

FILE file_; // dummy

FILE * open_file(const char *) {
    std::cout << "Call to open\n";
    return &file_; // dummy implementation
}
void close_file(FILE *) {
    std::cout << "Call to close\n";    
}

class FileCloser { // perfectible, as we will see
   FILE * file;
public:
   FileCloser(FILE *file) : file{ file } {
   }
   ~FileCloser() {
      close_file(file);
   }
};

int read_from(FILE *, char *, int ) {
    return 0; // dummy implementation
}

void process(const std::vector<char> &) {
}

enum { N = 2048 };

void f(const char *name) {
   FILE *file = open_file(name);
   if(!file) return; // failure
   FileCloser fc{ file }; // <-- fc manages file now
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
