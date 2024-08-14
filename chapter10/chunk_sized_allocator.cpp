// also available live: https://wandbox.org/permlink/MtdPNejNOAvbOBNq

#include <algorithm>
#include <vector>
#include <utility>
#include <memory>
#include <cassert>
#include <concepts>
#include <limits>
#include <array>
#include <iterator>
#include <mutex>

template <class T, std::same_as<T> ... Ts>
   constexpr std::array<T, sizeof...(Ts)+1> make_array(T n, Ts ... ns) {
      return { n, ns... };
   }

constexpr bool is_power_of_two(std::integral auto n) {
   return n && ((n & (n - 1)) == 0);
}

class integral_value_too_big {};

constexpr auto next_power_of_two(std::integral auto n) {
   constexpr auto upper_limit = std::numeric_limits<decltype(n)>::max();
   for(; n != upper_limit && !is_power_of_two(n); ++n)
       ;
   if(!is_power_of_two(n)) throw integral_value_too_big{};
   return n;
}

template <class T>
   constexpr bool is_sorted(const T &c) {
      return std::is_sorted(std::begin(c), std::end(c));
   }

template <int N, auto ... Sz>
   class ChunkSizedAllocator {
      static_assert(is_sorted(make_array(Sz...)));
      static_assert(sizeof...(Sz) > 0);
      static_assert(((Sz >= sizeof(std::max_align_t)) && ...));
      static_assert(N > 0);
      static constexpr unsigned long long sizes[] {
         next_power_of_two(Sz)...
      };
      using raw_ptr = void*;
      raw_ptr blocks[sizeof...(Sz)];
      int cur[sizeof...(Sz)] {}; // initialized to zero
      bool within_block(void *p, int i) {
         void* b = blocks[i];
         void* e = static_cast<char*>(b) + N * sizes[i];
         return p == b ||
                (std::less{}(b, p) && std::less{}(p, e));
      }
      std::mutex m;
   public:
      ChunkSizedAllocator(const ChunkSizedAllocator&) = delete;
      ChunkSizedAllocator& operator=(const ChunkSizedAllocator&) = delete;
      ChunkSizedAllocator() {
         int i = 0;
         for(auto sz : sizes)
            blocks[i++] = std::malloc(N * sz);
         assert(std::none_of(std::begin(blocks), std::end(blocks), [](auto p) { return !p; }));
      }
      ~ChunkSizedAllocator() {
         for(auto p : blocks)
            std::free(p);
      }
      auto allocate(std::size_t n) {
         // use smallest block available
         for(std::size_t i = 0; i != std::size(sizes); ++i) {
            if(n < sizes[i]) {
               std::lock_guard _ { m };
               if(cur[i] < N) {
                  void *p = static_cast<char*>(blocks[i]) + cur[i] * sizes[i];
                  ++cur[i];
                  return p;
               }
            }
         }
         // either no block fits or no block left
         return ::operator new(n);
      }
      void deallocate(void *p) {
         for(std::size_t i = 0; i != std::size(sizes); ++i) {
            if(within_block(p, i)) {
               //std::lock_guard _ { m };
               // if you want to reuse the memory, it's in blocks[i]
               return;
            }
         }
         // p not in our blocks
         ::operator delete(p);
      }
   };

template <int N, auto ... Sz>
   void *operator new (std::size_t n, ChunkSizedAllocator<N, Sz...> &chunks) {
      return chunks.allocate(n);
   }
template <int N, auto ... Sz>
   void operator delete (void *p, ChunkSizedAllocator<N, Sz...> &chunks) {
      return chunks.deallocate(p);
   }
// new[] and delete[] left as an exercise ;)


//
// Test program
//

#include <chrono>
#include <utility>
#include <functional>

template <class F, class ... Args>
   auto test(F f, Args &&... args) {
      using namespace std;
      using namespace std::chrono;
      auto pre = high_resolution_clock::now();
      auto res = f(std::forward<Args>(args)...);
      auto post = high_resolution_clock::now();
      return pair{ res, post - pre };
   }

template <int N> struct dummy { char _[N] {}; };

template <int N> auto test_dummy() {
   return std::pair<void *, std::function<void(void*)>>{
       new dummy<N>{}, [](void *p) { delete static_cast<dummy<N>*>(p); }
   };
}
template <int N, class T> auto test_dummy(T &alloc) {
   return std::pair<void *, std::function<void(void*)>>{
      new (alloc) dummy<N>{}, [&alloc](void *p) { ::operator delete(p, alloc); }
   };
}

#include <print>
#include <vector>
int main() {
   using namespace std;
   using namespace std::chrono;
   constexpr int N = 100'000;
   using Alloc = ChunkSizedAllocator<N, 32, 62 /* 64 */, 128>;
   Alloc chunks; // construct the ChunkSizedAllocator
   auto [r0, dt0] = test([ptrs = std::vector<std::pair<void*, std::function<void(void*)>>>(N * 3)]() mutable {
      // allocation
      for(int i = 0; i != N * 3; i += 3) {
         ptrs[i] = test_dummy<30>();
         ptrs[i + 1] = test_dummy<60>();
         ptrs[i + 2] = test_dummy<100>();
      }
      // cleanup
      for(auto & p : ptrs)
         p.second(p.first);
      return std::size(ptrs);
   });
   auto [r1, dt1] = test([&chunks, ptrs = std::vector<std::pair<void*, std::function<void(void*)>>>(N * 3)]() mutable {
      // allocation
      for(int i = 0; i != N * 3; i += 3) {
         ptrs[i] = test_dummy<30>(chunks);
         ptrs[i + 1] = test_dummy<60>(chunks);
         ptrs[i + 2] = test_dummy<100>(chunks);
      }
      // cleanup
      for(auto & p : ptrs)
         p.second(p.first);
      return std::size(ptrs);
   });
   std::print("Standard version : {}\n", duration_cast<microseconds>(dt0));
   std::print("Chunked version  : {}\n", duration_cast<microseconds>(dt1));
}
