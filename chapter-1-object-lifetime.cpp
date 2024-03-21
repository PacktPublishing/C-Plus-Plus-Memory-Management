// also available live: https://wandbox.org/permlink/DonyTvJAn7IGoNn0

#include <string>
#include <iostream>
#include <format>
using namespace std;
struct X {
   std::string s;
   X(std::string_view s) : s{ s } {
      std::cout << std::format("X::X({})\n", s);
   }
   ~X(){
      std::cout << std::format("~X::X() for {}\n", s);
   }
};
X glob { "glob" };
void g() {
   X xg{ "g()" };
}
int main() {
   X *p0 = new X{ "p0" };
   [[maybe_unused]] X *p1 = new X{ "p1" }; // will leak
   X xmain{ "main()" };
   g();
   delete p0;
   // oops, forgot delete p1
}
