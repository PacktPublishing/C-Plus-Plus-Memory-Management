// also available live: https://wandbox.org/permlink/FSKxAbSMt344cqu3

#include <iostream>
#include <atomic>
#include <utility>

namespace managing_memory_book {
   // naïve shared_ptr
   template <class T>
   class shared_ptr {
      T* p = nullptr;
      std::atomic<long long> *ctr = nullptr;
   public:
      shared_ptr() = default;
      shared_ptr(T* p) : p{ p } {
         if(p) try {
            ctr = new std::atomic<long long>{ 1LL };
         } catch(...) {
            delete p;
            throw;
         }
      }
      void swap(shared_ptr &other) noexcept {
         using std::swap;
         swap(p, other.p);
         swap(ctr, other.ctr);
      }
      shared_ptr(const shared_ptr &other) : p{ other.p }, ctr{ other.ctr } {
         if(ctr)
            ++(*ctr);
      }
      shared_ptr& operator=(const shared_ptr &other) {
         shared_ptr{ other }.swap(*this);
         return *this;
      }
      ~shared_ptr() {
         if(ctr) {
            auto expected = ctr->load();
            auto desired = expected - 1;
            while(ctr->compare_exchange_weak(expected, desired))
                desired = expected - 1;
            if(desired == 0) { // I was the last user of *p
               delete p;
               delete ctr;
            }
         }
      }
      shared_ptr(shared_ptr &&other) noexcept
         : p{ std::exchange(other.p, nullptr) },
           ctr{ std::exchange(other.ctr, nullptr) } {
      }
      shared_ptr& operator=(shared_ptr &&other) noexcept {
         shared_ptr{ std::move(other) }.swap(*this);
         return *this;
      }
      bool empty() const noexcept { return !p; }
      operator bool() const noexcept { return !empty(); }
      bool operator==(const shared_ptr &other) const noexcept {
         return p == other.p;
      }
      // inferred from operator==() since C++20
      bool operator!=(const shared_ptr &other) const noexcept {
         return !(*this == other);
      }
      T *get() noexcept { return p; }
      const T *get() const noexcept { return p; }
      T& operator*() noexcept { return *p; }
      const T& operator*() const noexcept { return *p; }
      T* operator->() noexcept { return p; }
      const T* operator->() const noexcept { return p; }
   };
}
#include <thread>
#include <chrono>
#include <random>
#include <iostream>
using namespace std::literals;
struct X {
   int n;
   X(int n) : n{ n } {}
   ~X() { std::cout << "X::~X()\n"; }
};
int main() {
   using managing_memory_book::shared_ptr;
   std::mt19937 prng{ std::random_device{}() };
   std::uniform_int_distribution<int> die{ 200, 300 };
   shared_ptr<X> p{ new X{ 3 } };
   using std::chrono::milliseconds; // shortcut
   std::thread th0{ [p, dt = die(prng)] {
      std::this_thread::sleep_for(milliseconds{dt});
      std::cout << "end of th0, p->n : " << p->n << '\n';
   } };
   std::thread th1{ [p, dt = die(prng)] {
      std::this_thread::sleep_for(milliseconds{dt});
      std::cout << "end of th1, p->n : " << p->n << '\n';
   } };
   th1.detach();
   th0.detach();
   std::this_thread::sleep_for(350ms);
   std::cout << "end main()\n";
}
