// also available live: https://wandbox.org/permlink/PfmioAXubSIFlyKi

#include <cstddef>
#include <algorithm>
#include <utility>
#include <initializer_list>
#include <iterator>
#include <cstdlib>
#include <memory>

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
   struct deleter { // HERE
      Vector& source;
      void operator()(value_type* p) {
         std::destroy(std::begin(source), std::end(source));
         std::free(static_cast<void*>(p));
      }
   };
   std::unique_ptr<value_type[], deleter> elems; // HERE
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
   iterator begin() { return elems.get(); } // HERE
   const_iterator begin() const { return elems.get(); } // HERE
   const_iterator cbegin() const { return begin(); }
   iterator end() { return begin() + size(); }
   const_iterator end() const { return begin() + size(); }
   const_iterator cend() const { return end(); }
   // HERE
   constexpr Vector()
      : elems{ nullptr, deleter { *this } } {
   }
   // HERE
   Vector(size_type n, const_reference init)
      : elems{
           static_cast<pointer>(std::malloc(n * sizeof(value_type))),
           deleter{ *this }
      } {
      std::uninitialized_fill(begin(), begin() + n, init); // HERE
      nelems = cap = n; // HERE
   }
   // HERE
   Vector(const Vector& other)
      : elems{
           static_cast<pointer>(std::malloc(other.size() * sizeof(value_type))),
           deleter{ *this }
      } {
      std::uninitialized_copy(other.begin(), other.end(), begin());
      nelems = cap = other.size(); // HERE
   }
   // HERE
   Vector(Vector&& other) noexcept
      : elems{ std::exchange(other.elems.release()), deleter{ *this } },
      nelems{ std::exchange(other.nelems, 0) },
      cap{ std::exchange(other.cap, 0) } {
   }
   // HERE
   Vector(std::initializer_list<T> src)
      : elems{
           static_cast<pointer>(std::malloc(src.size() * sizeof(value_type))),
           deleter { *this }
      } {
      std::uninitialized_copy(src.begin(), src.end(), begin());
      nelems = cap = src.size(); // HERE
   }
   // HERE
   ~Vector() = default;
   // ...
   void swap(Vector& other) noexcept {
      using std::swap;
      swap(elems, other.elems); // HERE
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
   reference operator[](size_type n) { return elems[n]; } // HERE
   const_reference operator[](size_type n) const { return elems[n]; } // HERE
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
      std::construct_at(end(), val);
      ++nelems;
   }
   void push_back(T&& val) {
      if (full())
         grow();
      std::construct_at(end(), std::move(val));
      ++nelems;
   }
   template <class ... Args>
   reference emplace_back(Args &&...args) {
      if (full())
         grow();
      std::construct_at(end(), std::forward<Args>(args)...);
      ++nelems;
      return back();
   }
private:
   void grow() {
      reserve(capacity() ? capacity() * 2 : 16);
   }
public:
   // HERE
   void reserve(size_type new_cap) {
      if (new_cap <= capacity()) return;
      std::unique_ptr<value_type[]> p{
         static_cast<pointer>(std::malloc(new_cap * sizeof(T)))
      };
      if constexpr (std::is_nothrow_move_assignable_v<T>) {
         std::uninitialized_move(begin(), end(), p.get());
      } else {
         std::uninitialized_copy(begin(), end(), p.get());
      }
      std::destroy(begin(), end());
      elems.reset(p.release());
      cap = new_cap;
   }
   // HERE
   void resize(size_type new_cap) {
      if (new_cap <= capacity()) return;
      std::unique_ptr<value_type[]> p =
         static_cast<pointer>(std::malloc(new_cap * sizeof(T)));
      if constexpr (std::is_nothrow_move_assignable_v<T>) {
         std::uninitialized_move(begin(), end(), p.get());
      } else {
         std::uninitialized_copy(begin(), end(), p.get());
      }
      std::uninitialized_fill(p.get() + size(), p.get() + new_cap, value_type{});
      std::destroy(begin(), end());
      elems.reset(p.release());
      nelems = cap = new_cap;
   }
   // etc.
   // two small examples, one that inserts elements
   // at a given position in the container and one
   // that erases an element at a given position
   // in the container
   template <class It>
   iterator insert(const_iterator pos, It first, It last) {
      iterator pos_ = const_cast<iterator>(pos);
      const auto remaining = capacity() - size();
      const auto n = std::distance(first, last);
      if (std::cmp_less(remaining, n)) {
         auto index = std::distance(begin(), pos_);
         reserve(capacity() + n - remaining);
         pos_ = std::next(begin(), index);
      }

      const auto nb_to_uninit_displace =
         std::min<std::ptrdiff_t>(n, end() - pos_);
      auto where_to_uninit_displace = end() + n - nb_to_uninit_displace;
      if constexpr (std::is_nothrow_move_constructible_v<T>)
         std::uninitialized_move(end() - nb_to_uninit_displace, end(),
                                 where_to_uninit_displace);
      else
         std::uninitialized_copy(end() - nb_to_uninit_displace, end(),
                                 where_to_uninit_displace);

      // note : might be zero
      const auto nb_to_uninit_insert =
         std::max<std::ptrdiff_t>(0, n - nb_to_uninit_displace);
      auto where_to_uninit_insert = end();
      std::uninitialized_copy(last - nb_to_uninit_insert, last,
                              where_to_uninit_insert);

      // note : might be zero
      const auto nb_to_backward_displace =
         std::max<std::ptrdiff_t>(0, end() - pos_ - nb_to_uninit_displace);
      auto where_to_backward_displace = end(); // note : end of destination
      if constexpr (std::is_nothrow_move_assignable_v<T>)
         std::move_backward(pos_, pos_ + nb_to_backward_displace,
                            where_to_backward_displace);
      else
         std::copy_backward(pos_, pos_ + nb_to_backward_displace,
                            where_to_backward_displace);

      std::copy(first, first + n - nb_to_uninit_insert, pos_);
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

#include <string>
int main() {
   // insertions at the beginning
   for (int i = 0; i != 10; ++i) {
      Vector v0{ 2,3,5,7,11 };
      Vector v1(i, -1);
      std::cout << std::string(70, '-') << '\n';
      std::cout << "Inserting " << i << " elements at the beginning...\n";
      std::cout << "Before : " << v0 << '\n';
      v0.insert(v0.begin(), v1.begin(), v1.end());
      std::cout << "After  : " << v0 << '\n';
   }
   // insertions at the end
   for (int i = 0; i != 10; ++i) {
      Vector v0{ 2,3,5,7,11 };
      Vector v1(i, -1);
      std::cout << std::string(70, '-') << '\n';
      std::cout << "Inserting " << i << " elements at the end...\n";
      std::cout << "Before : " << v0 << '\n';
      v0.insert(v0.end(), v1.begin(), v1.end());
      std::cout << "After  : " << v0 << '\n';
   }
   // insertions at position 2
   for (int i = 0; i != 10; ++i) {
      Vector v0{ 2,3,5,7,11 };
      Vector v1(i, -1);
      std::cout << std::string(70, '-') << '\n';
      std::cout << "Inserting " << i << " elements at the middle...\n";
      std::cout << "Before : " << v0 << '\n';
      v0.insert(std::next(v0.begin(), 2), v1.begin(), v1.end());
      std::cout << "After  : " << v0 << '\n';
   }
}
