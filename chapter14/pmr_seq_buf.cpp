// also available live: https://wandbox.org/permlink/YsE8PwrbnP7G6Q0A

#include <print>
#include <vector>
#include <string>
#include <memory_resource>
int main() {
   enum { N = 10'000 };
   alignas(int) char buf[N * sizeof(int)]{};
   std::pmr::monotonic_buffer_resource
      res{ std::begin(buf), std::size(buf) };
   std::pmr::vector<int> v{ &res };
   v.reserve(N);
   for (int i = 0; i != N; ++i)
      v.emplace_back(i + 1);
   for (auto n : v)
      std::print("{} ", n);
   std::print("\n {}\n", std::string(70, '-'));
   for (char * p = buf; p != buf + std::size(buf); p += sizeof(int))
      std::print("{} ", *reinterpret_cast<int*>(p));
}