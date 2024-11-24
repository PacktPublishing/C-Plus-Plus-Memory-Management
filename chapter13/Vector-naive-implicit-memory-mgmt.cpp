// also available live: https://wandbox.org/permlink/b4eoON73dzm01K6P

#include <cstddef>
#include <algorithm>
#include <utility>
#include <initializer_list>
#include <iterator>
#include <memory> // <--

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
   std::unique_ptr<value_type[]> elems; // <--
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
   iterator begin() { return elems.get(); } // <--
   const_iterator begin() const { return elems.get(); } // <--
   const_iterator cbegin() const { return begin(); }
   iterator end() { return begin() + size(); }
   const_iterator end() const { return begin() + size(); }
   const_iterator cend() const { return end(); }
   // ...
   Vector() = default;
   Vector(size_type n, const_reference init)
      : elems{ new value_type[n] }, nelems{ n }, cap{ n } {
      std::fill(begin(), end(), init);
   }
   // ...
   Vector(const Vector& other)
      : elems{ new value_type[other.size()] }, nelems{ other.size() }, cap{ other.size() } {
      std::copy(other.begin(), other.end(), begin());
   }
   Vector(Vector&& ) = default;
   Vector(std::initializer_list<T> src)
      : elems{ new value_type[src.size()] }, nelems{ src.size() }, cap{ src.size() } {
      std::copy(src.begin(), src.end(), begin());
   }
   // ...
   ~Vector() = default;
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
   Vector& operator=(Vector&& ) = default;
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
      elems[size()] = val;
      ++nelems;
   }
   void push_back(T&& val) {
      if (full())
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
   void grow() { resize(capacity() * 2); }
public:
   void resize(size_type new_cap) {
      if(new_cap <= capacity()) return;
      auto p = std::make_unique<value_type[]>(new_cap);
      if constexpr(std::is_nothrow_move_assignable_v<T>) {
         std::move(begin(), end(), p.get());         
      } else {
         std::copy(begin(), end(), p.get());
      }
      elems.reset(p.release());
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
         resize(capacity() + n - remaining);
         pos_ = std::next(begin(), index);
      }
      std::copy_backward(pos_, end(), end() + n);
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

template <class T>
std::ostream& operator<<(std::ostream& os, const Vector<T>& v) {
   if (v.empty()) return os;
   os << v.front();
   for (auto p = std::next(v.begin()); p != v.end(); ++p)
      os << ',' << *p;
   return os;
}

int main() {
   Vector v0{ 2,3,5,7,11 };
   Vector v1 = v0; // copy ctor
   std::cout << v1 << '\n'; // 2,3,5,7,11
   for (int n : { 13, 17, 19, 23, 29, 31, 37, 41, 43, 47 })
      v0.push_back(n); // will call grow() at some point
   // Size: 15, capacity: 20
   // 2,3,5,7,11,13,17,19,23,29,31,37,41,43,47
   std::cout << "Size: " << v0.size() << ", capacity: " << v0.capacity() << '\n'
      << v0 << '\n';
   int arr[]{ -2, -3, -4 };
   v1.insert(v1.begin(), std::begin(arr), std::end(arr));
   // Size: 8, capacity: 8
   // -2,-3,-4,2,3,5,7,11
   std::cout << "Size: " << v1.size() << ", capacity: " << v1.capacity() << '\n'
      << v1 << '\n';
   v1.insert(v1.end(), std::begin(arr), std::end(arr));
   // Size: 11, capacity: 11
   // -2,-3,-4,2,3,5,7,11,-2,-3-,4
   std::cout << "Size: " << v1.size() << ", capacity: " << v1.capacity() << '\n'
      << v1 << '\n';
   v1.erase(std::next(v1.begin(), 2));
   // Size: 10, capacity: 11
   // -2,-3,2,3,5,7,11,-2,-3,-4
   std::cout << "Size: " << v1.size() << ", capacity: " << v1.capacity() << '\n'
      << v1 << '\n';
}
