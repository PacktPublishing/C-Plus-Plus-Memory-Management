// also available live: https://wandbox.org/permlink/ja9o1as9mnWhHnlq

#include <chrono>
#include <iostream>
#include <thread>
int f() {
   using namespace std::literals;
   std::this_thread::sleep_for(500ms);
   return 3;
}
int main() {
   using std::cout;
   using namespace std::chrono;
   auto pre = system_clock::now();
   int res = f();
   auto post = system_clock::now();
   cout << "Computed " << res << " in "
        << duration_cast<microseconds>(post - pre);
}
