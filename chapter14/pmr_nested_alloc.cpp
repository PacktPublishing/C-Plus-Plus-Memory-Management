// also available live: https://wandbox.org/permlink/5XDYMF4vIZZjM0E8

#include <print>
#include <vector>
#include <string>
#include <memory_resource>
int main() {
   auto make_str = [](const char *p, int n) -> std::pmr::string {
      auto s = std::string{ p } + std::to_string(n);
      return { std::begin(s), std::end(s) };
   };
   enum { N = 2'000 };
   alignas(std::pmr::string) char buf[N]{};
   std::pmr::monotonic_buffer_resource
      res{ std::begin(buf), std::size(buf) };
   std::pmr::vector<std::pmr::string> v{ &res };
   for (int i = 0; i != 10; ++i)
      v.emplace_back(make_str("I love my instructor ", i));
   for (const auto &s : v)
      std::print("{} ", s);
   std::print("\n {}\n", std::string(70, '-'));
   for (char c : buf)
      std::print("{} ", c);
}