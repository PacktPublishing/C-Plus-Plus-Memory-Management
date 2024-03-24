// also available live: https://wandbox.org/permlink/LG38P4FDKq8h8xwq

// inspired from a cppreference example
#include <iostream>
#include <memory>
#include <format>
void observe(std::weak_ptr<int> w) {
   if (std::shared_ptr<int> sh = w.lock())
      std::cout << std::format("*sh == {}\n", *sh);
   else
      std::cout << "w is expired\n";
} 
int main() {
   std::weak_ptr<int> w;
   {
      auto sh = std::make_shared<int>(3);
      w = sh;
      // w points to a live shared_ptr<int> here
      observe(w);
   }
   // w points to an expired shared_ptr<int> here
   observe(w);
}
