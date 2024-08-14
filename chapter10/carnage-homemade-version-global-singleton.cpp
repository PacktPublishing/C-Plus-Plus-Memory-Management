// also available live: https://wandbox.org/permlink/eeg4Q4T1kws1Os1K

#ifndef ORC_H
#define ORC_H

//
// Orc.h
//

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

#include <cassert>
#include <cstdlib>
#include <mutex>

class Tribe {
   std::mutex m;
   char *p, *cur;
   Tribe() : p{ static_cast<char*>(std::malloc(Orc::NB_MAX * sizeof(Orc))) } {
      assert(p);
      cur = p;
   }
   Tribe(const Tribe&) = delete;
   Tribe& operator=(const Tribe&) = delete;
   static Tribe singleton;
public:
   ~Tribe() {
      std::free(p);
   }
   static auto &get() {
      return singleton;
   }
   void * allocate() {
      std::lock_guard _ { m };
      auto q = cur;
      cur += sizeof(Orc);
      return q;
   }
   void deallocate(void *) noexcept {
   }
};

Tribe Tribe::singleton;
       
void * Orc::operator new(std::size_t) {
   return Tribe::get().allocate();
}
void * Orc::operator new[](std::size_t) {
   assert(false);
}
void Orc::operator delete(void *p) noexcept {
   Tribe::get().deallocate(p);
}
void Orc::operator delete[](void *) noexcept {
   assert(false);
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
