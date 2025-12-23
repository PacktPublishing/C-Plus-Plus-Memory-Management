// also available live: https://wandbox.org/permlink/2HlHX00xWs0QKEEM

#include <cstddef>
#include <algorithm>
#include <utility>
#include <initializer_list>
#include <iterator>

template <class T>
   class Vector {
   public:
      using value_type = T;
      using size_type = std::size_t;
      using pointer = T*;
      using const_pointer = const T*;
      using reference = T&;
      using const_reference = const T&;
   private:
      pointer elems{};
      size_type nelems{},
                cap{};
      // ...
   public:
      size_type size() const { return nelems; }
      size_type capacity() const { return cap; }
      bool empty() const { return size() == 0; }
   private:
      bool full() const { return size() == capacity(); }
      // ...
   public:
      using iterator = pointer;
      using const_iterator = const_pointer;
      template <class S>
         auto begin(this S && self) { return self.elems; }
      template <class S>
         auto end(this S && self) { return self.begin() + self.size(); }
      const_iterator cbegin() const { return begin(); }
      const_iterator cend() const { return end(); }
      // ...
      Vector() = default;
      Vector(size_type n, const_reference init)
         : elems{ new value_type[n] }, nelems{ n }, cap{ n } {
         try {
            std::fill(begin(), end(), init);
         } catch(...) {
            delete [] elems;
            throw;
         }
      }
      // ...
      Vector(const Vector &other)
         : elems{ new value_type[other.size()] }, nelems{ other.size() }, cap{ other.size() } {
         try {
            std::copy(other.begin(), other.end(), begin());
         } catch(...) {
            delete [] elems;
            throw;
         }
      }
      // ...
      Vector(Vector &&other) noexcept
         : elems{ std::exchange(other.elems, nullptr) },
           nelems{ std::exchange(other.nelems, 0) },
           cap{ std::exchange(other.cap, 0) } {
      }
      // ...
      Vector(std::initializer_list<T> src)
         : elems{ new value_type[src.size()] }, nelems {src.size() }, cap{ src.size() } {
         try {
            std::copy(src.begin(), src.end(), begin());
         } catch(...) {
            delete [] elems;
            throw;
         }
      }
      // ...
      ~Vector() {
         delete [] elems;
      }
      // ...
      void swap(Vector &other) noexcept {
         using std::swap;
         swap(elems, other.elems);
         swap(nelems, other.nelems);
         swap(cap, other.cap);
      }
      Vector& operator=(const Vector &other) {
         Vector{ other }.swap(*this);
         return *this;
      }
      Vector& operator=(Vector &&other) {
         Vector{ std::move(other) }.swap(*this);
         return *this;
      }
      // ...
      template <class S>
         decltype(auto) operator[](this S && self, size_type n) {
            return self.elems[n];
         }
      // precondition: !empty()
      template <class S>
         decltype(auto) front(this S &&self) { return self[0]; }
      template <class S>
         decltype(auto) back(this S &&self) { return self[self.size()-1]; }
      // ...
      bool operator==(const Vector &other) const {
         return size() == other.size() &&
                std::equal(begin(), end(), other.begin());
      }
      // ...
      void push_back(const_reference val) {
         if(full())
            grow();
         elems[size()] = val;
         ++nelems;
      }
      void push_back(T &&val) {
         if(full())
            grow();
         elems[size()] = std::move(val);
         ++nelems;
      }
      template <class ... Args>
         reference emplace_back(Args &&...args) {
            if (full())
               grow();
            elems[size()] = value_type(std::forward<Args>(args)...);
            ++nelems;
            return back();
         }
   private:
      void grow() { resize(capacity()? capacity() * 2 : 16); }
   public:
      void resize(size_type new_cap) {
         if (new_cap <= capacity()) {
            nelems = new_cap;
            return;
         }
         auto p = new T[new_cap];
         if constexpr(std::is_nothrow_move_assignable_v<T>) {
            std::move(begin(), end(), p);         
         } else try {
            std::copy(begin(), end(), p);
         } catch (...) {
            delete[] p;
            throw;
         }
         delete[] elems;
         elems = p;
         cap = new_cap;
      }
      // etc.
   };

#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include <utility>
#include <format>
#include <print>
using namespace std;
using namespace std::chrono;

template <class F, class ... Args>
   auto test(F f, Args &&... args) {
      auto pre = high_resolution_clock::now();
      auto res = f(std::forward<Args>(args)...);
      auto post = high_resolution_clock::now();
      return pair{ res, post - pre };
   }

int main() {
   const int N0 = 1'000'000;
   const int N1 = 100'000;
   auto [r0,dt0] = test([N = N0] {
      Vector<int> v;
      for(int i = 0; i != N; ++i)
         v.push_back(i);
      return v.size();
   });
   auto [r1,dt1] = test([N = N0] {
      std::vector<int> v;
      for(int i = 0; i != N; ++i)
         v.push_back(i);
      return v.size();
   });
   auto [r2,dt2] = test([N = N1] {
      Vector<string> v;
      for(int i = 0; i != N; ++i)
         v.push_back("I love this book, strange as it is");
      return v.size();
   });
   auto [r3,dt3] = test([N = N1] {
      std::vector<string> v;
      for(int i = 0; i != N; ++i)
         v.push_back("I love this book, strange as it is");
      return v.size();
   });
   print("Vector<int>,         push_back(), {} times: {} in {}\n",
         N0, r0, duration_cast<microseconds>(dt0));
   print("std::vector<int>,    push_back(), {} times: {} in {}\n",
         N0, r1, duration_cast<microseconds>(dt1));
   print("Vector<string>,      push_back(), {} times: {} in {}\n",
         N1, r2, duration_cast<microseconds>(dt2));
   print("std::vector<string>, push_back(), {} times: {} in {}\n",
         N1, r3, duration_cast<microseconds>(dt3));
}
