// also available live: https://wandbox.org/permlink/A1Cd3A4sIgoGTIKt

//
// SizeBasedArena.h
//

#ifndef SIZE_BASED_ARENA_H
#define SIZE_BASED_ARENA_H

#include <cassert>
#include <cstdlib>
#include <mutex>

template <class T, std::size_t N>
class SizeBasedArena {
   std::mutex m;
   char *p, *cur;
   SizeBasedArena()
      : p{ static_cast<char*>(std::malloc(N * sizeof(T))) } {
      assert(p);
      cur = p;
   }
   SizeBasedArena(const SizeBasedArena&) = delete;
   SizeBasedArena& operator=(const SizeBasedArena&) = delete;
public:
   ~SizeBasedArena() {
      std::free(p);
   }
   static auto &get() {
      static SizeBasedArena singleton;
      return singleton;
   }
   void * allocate_one() {
      std::lock_guard _ { m };
      auto q = cur;
      cur += sizeof(T);
      return q;
   }
   void * allocate_n(std::size_t n) {
      std::lock_guard _ { m };
      auto q = cur;
      cur += n * sizeof(T);
      return q;
   }
   void deallocate_one(void *) noexcept {
   }
   void deallocate_n(void *) noexcept {
   }
};

#endif


//
// Orc.h
//

#ifndef ORC_H
#define ORC_H

#define HOMEMADE_VERSION

#include <cstddef>
#include <new>

class Orc {
   char name[4]{ 'U', 'R', 'G' };
   int strength = 100;
   double smell = 1000.0;
public:
   static constexpr int NB_MAX = 1'000'000;
#ifdef HOMEMADE_VERSION
   void * operator new(std::size_t);
   void * operator new[](std::size_t);
   void operator delete(void *) noexcept;
   void operator delete[](void *) noexcept;
#endif
};

#endif

//
// Orc.cpp
//
// #include "Orc.h"

#ifdef HOMEMADE_VERSION

// #include "SizeBasedArena.h"

using Tribe = SizeBasedArena<Orc, Orc::NB_MAX>;
       
void * Orc::operator new(std::size_t) {
   return Tribe::get().allocate_one();
}
void * Orc::operator new[](std::size_t n) {
   return Tribe::get().allocate_n(n / sizeof(Orc));
}
void Orc::operator delete(void *p) noexcept {
   Tribe::get().deallocate_one(p);
}
void Orc::operator delete[](void *p) noexcept {
   Tribe::get().deallocate_n(p);
}

#endif

//
// Test program
//
// #include "Orc.h"

#include <chrono>
#include <utility>

template <class F, class ... Args>
   auto test(F f, Args &&... args) {
      using namespace std;
      using namespace std::chrono;
      auto pre = high_resolution_clock::now();
      auto res = f(std::forward<Args>(args)...);
      auto post = high_resolution_clock::now();
      return pair{ res, post - pre };
   }

#include <print>
#include <vector>
int main() {
   using namespace std;
   using namespace std::chrono;
#ifdef HOMEMADE_VERSION
   print("HOMEMADE VERSION\n");
#else
   print("STANDARD LIBRARY VERSION\n");
#endif
   vector<Orc*> orcs;
   orcs.reserve(Orc::NB_MAX);
   auto [r0, dt0] = test([&orcs] {
      for(int i = 0; i != Orc::NB_MAX; ++i)
         orcs.push_back(new Orc);
      return size(orcs);
   });
   // ...
   // CARNAGE (CENSORED)
   // ...
   auto [r1, dt1] = test([&orcs] {
      for(auto p : orcs)
         delete p;
      return size(orcs);
   });
   print("Construction: {} orcs in {}\n", size(orcs), duration_cast<microseconds>(dt0));
   print("Destruction:  {} orcs in {}\n", size(orcs), duration_cast<microseconds>(dt1));
}
