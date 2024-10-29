// also available live: https://wandbox.org/permlink/u4poOOY4mARuoWxX

#include <cstddef>
#include <algorithm>
#include <utility>
#include <initializer_list>
#include <iterator>
#include <cstdlib> // HERE
#include <memory> // HERE

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
   iterator begin() { return elems; }
   const_iterator begin() const { return elems; }
   const_iterator cbegin() const { return begin(); }
   iterator end() { return begin() + size(); }
   const_iterator end() const { return begin() + size(); }
   const_iterator cend() const { return end(); }
   // ...
   Vector() = default;
   // HERE
   Vector(size_type n, const_reference init)
      : elems{ static_cast<pointer>(std::malloc(n * sizeof(value_type))) },
        nelems{ n }, cap{ n } {
      
      try {
         std::uninitialized_fill(begin(), end(), init);
      } catch (...) {
         std::free(elems);
         throw;
      }
   }
   // HERE
   Vector(const Vector& other)
      : elems{ static_cast<pointer>(std::malloc(other.size() * sizeof(value_type))) },
        nelems{ other.size() }, cap{ other.size() } {
      try {
         std::uninitialized_copy(other.begin(), other.end(), begin());
      } catch (...) {
         std::free(elems);
         throw;
      }
   }
   // ...
   Vector(Vector&& other) noexcept
      : elems{ std::exchange(other.elems, nullptr) },
        nelems{ std::exchange(other.nelems, 0) },
        cap{ std::exchange(other.cap, 0) } {
   }
   // ...
   // HERE
   Vector(std::initializer_list<T> src)
      : elems{ static_cast<pointer>(std::malloc(src.size() * sizeof(value_type))) },
        nelems{ src.size() }, cap{ src.size() } {
      try {
         std::copy(src.begin(), src.end(), begin());
      } catch (...) {
         std::free(elems);
         throw;
      }
   }
   // HERE
   ~Vector() {
      std::destroy(begin(), end());
      std::free(elems);
   }
   // ...
   void swap(Vector& other) noexcept {
      using std::swap;
      swap(elems, other.elems);
      swap(nelems, other.nelems);
      swap(cap, other.cap);
   }
   Vector& operator=(const Vector& other) {
      Vector{ other }.swap(*this);
      return *this;
   }
   Vector& operator=(Vector&& other) {
      Vector{ std::move(other) }.swap(*this);
      return *this;
   }
   // ...
   reference operator[](size_type n) { return elems[n]; }
   const_reference operator[](size_type n) const { return elems[n]; }
   // precondition: !empty()
   reference front() { return (*this)[0]; }
   const_reference front() const { return (*this)[0]; }
   reference back() { return (*this)[size() - 1]; }
   const_reference back() const { return (*this)[size() - 1]; }
   // ...
   bool operator==(const Vector& other) const {
      return size() == other.size() &&
         std::equal(begin(), end(), other.begin());
   }
   // can be omitted since C++20
   bool operator!=(const Vector& other) const {
      return !(*this == other);
   }
   // ...
   void push_back(const_reference val) {
      if (full())
         grow();
      // HERE
      std::construct_at(end(), val);
      ++nelems;
   }
   void push_back(T&& val) {
      if (full())
         grow();
      // HERE
      std::construct_at(end(), std::move(val));
      ++nelems;
   }
   template <class ... Args>
      reference emplace_back(Args &&...args) {
         if (full())
            grow();
         // HERE
         std::construct_at(end(), std::forward<Args>(args)...);
         ++nelems;
         return back();
      }
private:
   // HERE
   void grow() {
      reserve(capacity()? capacity() * 2 : 16);
   }
public:
   // HERE
   void reserve(size_type new_cap) {
      if(new_cap <= capacity()) return;
      auto p = static_cast<pointer>(std::malloc(new_cap * sizeof(T)));
      if constexpr(std::is_nothrow_move_assignable_v<T>) {
         std::uninitialized_move(begin(), end(), p);         
      } else try {
         std::uninitialized_copy(begin(), end(), p);
      } catch (...) {
         std::free(p);
         throw;
      }
      std::destroy(begin(), end());
      std::free(elems);
      elems = p;
      cap = new_cap;
   }
   // HERE
   void resize(size_type new_cap) {
      if(new_cap <= capacity()) return;
      auto p = static_cast<pointer>(std::malloc(new_cap * sizeof(T)));
      if constexpr(std::is_nothrow_move_assignable_v<T>) {
         std::uninitialized_move(begin(), end(), p);         
      } else try {
         std::uninitialized_copy(begin(), end(), p);
      } catch (...) {
         std::free(p);
         throw;
      }
      std::uninitialized_fill(p + size(), p + capacity(), value_type{});
      std::destroy(begin(), end());
      std::free(elems);
      elems = p;
      nelems = new_cap;
      cap = new_cap;
   }
   // etc.
   // two small examples, one that inserts elements
   // at a given position in the container and one
   // that erases an element at a given position
   // in the container
   template <class It>
   iterator insert(const_iterator pos, It first, It last) {
      iterator pos_ = const_cast<iterator>(pos);
      // deliberate usage of unsigned integrals
      const std::size_t remaining = capacity() - size();
      const std::size_t n = std::distance(first, last);
      if (remaining < n) {
         auto index = std::distance(begin(), pos_);
         reserve(capacity() + n - remaining);
         pos_ = std::next(begin(), index);
      }
      auto m = std::distance(std::next(pos_, n), end());
      if (m > 0) {
         std::uninitialized_copy(pos_ + n, end(), end() + n - m);
         std::uninitialized_copy(pos_ + m, pos_ + n, end());
         std::copy_backward(pos_, pos_ + m, pos_ + n + m);
      }
      std::copy(first, last, pos_);
      nelems += n;
      return pos_;
   }
   iterator erase(const_iterator pos) {
      iterator pos_ = const_cast<iterator>(pos);
      if (pos_ == end()) return pos_;
      std::copy(std::next(pos_), end(), pos_);
      *std::prev(end()) = {};
      --nelems;
      return pos_;
   }
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
