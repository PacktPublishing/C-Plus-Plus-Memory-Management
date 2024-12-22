// also available live: https://wandbox.org/permlink/oFpuydyqtF85cx2p

#include <cstddef>
#include <algorithm>
#include <utility>
#include <initializer_list>
#include <iterator>
#include <cstdlib>
#include <memory>
#include <limits>

// note: would be more conformant if destruction was done in
// reverse order of construction
// note: takes A by reference deliberately
template <class A, class IIt, class T>
void uninitialized_fill_with_allocator(A& alloc, IIt bd, IIt ed, T init) {
   auto p = bd;
   try {
      for (; p != ed; ++p)
         alloc.construct(p, init); // <--
   } catch (...) {
      for (auto q = bd; q != p; ++q)
         alloc.destroy(q); // <--
      throw;
   }
}

// note: would be more conformant if destruction was done in
// reverse order of construction
// note: takes A by reference deliberately
template <class A, class IIt, class OIt>
void uninitialized_copy_with_allocator(A& alloc, IIt bs, IIt es, OIt bd) {
   auto p = bd;
   try {
      for (auto q = bs; q != es; ++q) {
         alloc.construct(p, *q); // <--
         ++p;
      }
   } catch (...) {
      for (auto q = bd; q != p; ++q)
         alloc.destroy(q); // <--
      throw;
   }
}

// note: would be more conformant if destruction was done in
// reverse order of construction
// note: takes A by reference deliberately
template <class A, class IIt, class OIt>
void uninitialized_move_with_allocator(A& alloc, IIt bs, IIt es, OIt bd) {
   auto p = bd;
   try {
      for (auto q = bs; q != es; ++q) {
         alloc.construct(p, std::move(*q)); // <--
         ++p;
      }
   } catch (...) {
      for (auto q = bd; q != p; ++q)
         alloc.destroy(q); // <--
      throw;
   }
}

// note: takes A by reference deliberately
template <class A, class It>
   void destroy_with_allocator(A &alloc, It b, It e) {
      for (; b != e; ++b)
         alloc.destroy(b);
   }

// note: std::cmp_less() requires C++20; this is a
// poor person's approximation
template<class T, class U>
   constexpr bool cmp_less(T a, U b) noexcept {
      if constexpr (std::is_signed_v<T> == std::is_signed_v<U>)
         return a < b;
      else if constexpr (std::is_signed_v<T>)
         return a < 0 || std::make_unsigned_t<T>(a) < b;
      else
         return b >= 0 && a < std::make_unsigned_t<U>(b);
   }

template <class T>
struct small_allocator {
   using value_type = T;
   using pointer = T*;
   using const_pointer = const T*;
   using reference = T&;
   using const_reference = const T&;
   using size_type = std::size_t;
   using difference_type = std::ptrdiff_t;
   constexpr size_type max_size() const {
      return std::numeric_limits<size_type>::max(); // bah
   }
   template <class U>
   struct rebind {
      using other = small_allocator<U>;
   };
   constexpr pointer address(reference r) const {
      return std::addressof(r);
   }
   constexpr const_pointer address(const_reference r) const {
      return std::addressof(r);
   }
   pointer allocate(size_type n) {
      auto p = static_cast<pointer>(malloc(n * sizeof(value_type)));
      if (!p) throw std::bad_alloc{};
      return p;
   }
   void deallocate(pointer p, size_type) {
      free(p);
   }
   void construct(pointer p, const_reference r) {
      new (static_cast<void*>(p)) value_type(r);
   }
   void destroy(const_pointer p) {
      if(p) p->~value_type();
   }
};

template <class T, class U>
constexpr bool operator==(const small_allocator<T>&, const small_allocator<U>&) {
   return true;
}
template <class T, class U>
constexpr bool operator!=(const small_allocator<T>&, const small_allocator<U>&) {
   return false;
}


//////////////////////////

#include <cstdint>

template <class T>
struct seq_buf_allocator {
   using value_type = T;
   using pointer = T*;
   using const_pointer = const T*;
   using reference = T&;
   using const_reference = const T&;
   using size_type = std::size_t;
   using difference_type = std::ptrdiff_t;
   constexpr size_type max_size() const {
      return std::numeric_limits<size_type>::max(); // bah
   }
//private:
   char* buf;
   pointer cur;
   size_type cap;
//public:
   seq_buf_allocator(char* buf, size_type cap) noexcept
      : buf{ buf }, cap{ cap } {
      cur = reinterpret_cast<pointer>(buf);
   }
   template <class U>
      seq_buf_allocator(const seq_buf_allocator<U> &other)
         : buf{ other.buf }, cap{ other.cap } {
         cur = reinterpret_cast<pointer>(buf);
      }
   template <class U>
   struct rebind {
      using other = seq_buf_allocator<U>;
   };
   constexpr pointer address(reference r) const {
      return std::addressof(r);
   }
   constexpr const_pointer address(const_reference r) const {
      return std::addressof(r);
   }
   pointer allocate(size_type n) {
      auto request = reinterpret_cast<std::intptr_t>(cur + n),
         limit = reinterpret_cast<std::intptr_t>(buf + cap);
      if (request > limit) { // <--
         throw std::bad_alloc{}; // <--
      }
      auto q = cur;
      cur += n; // <--
      return q;
   }
   void deallocate(pointer, size_type) { // <--
   }
   void construct(pointer p, const_reference r) {
      new (static_cast<void*>(p)) value_type(r);
   }
   void destroy(const_pointer p) {
      if (p) p->~value_type();
   }
};

//////////////////////////


template <class T, class A = std::allocator<T>>
class Vector : A { // note: private inheritance
public:
   using value_type = typename A::value_type;
   using size_type = typename A::size_type;
   using pointer = typename A::pointer;
   using const_pointer = typename A::const_pointer;
   using reference = typename A::reference;
   using const_reference = typename A::const_reference;
private:
   using A::allocate;
   using A::deallocate;
   using A::construct;
   using A::destroy;
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
   Vector() = default;
   // HERE
   Vector(A &alloc) : A{ alloc } {
   }
   Vector(size_type n, const_reference init)
      : A{}, elems{ allocate(n) }, nelems{ n }, cap{ n } { // <--
      try {
         uninitialized_fill_with_allocator(
            *static_cast<A*>(this), begin(), end(), init
         );
      } catch (...) {
         deallocate(elems, capacity()); // <--
         throw;
      }
   }
   // HERE
   Vector(const Vector& other)
      : A{},
      elems{ allocate(other.size()) }, // <--
      nelems{ other.size() }, cap{ other.size() } {
      try {
         uninitialized_copy_with_allocator(
            *static_cast<A*>(this), other.begin(), other.end(), begin()
         );
      } catch (...) {
         deallocate(elems, capacity()); // <--
         throw;
      }
   }
   // HERE
   Vector(Vector&& other) noexcept
      : A{},
      elems{ std::exchange(other.elems, nullptr) },
      nelems{ std::exchange(other.nelems, 0) },
      cap{ std::exchange(other.cap, 0) } {
   }
   // HERE
   Vector(std::initializer_list<T> src)
      : A{},
        elems{ allocate(src.size()) }, // <--
        nelems{ src.size() }, cap{ src.size() } {
      try {
         uninitialized_copy_with_allocator(
            *static_cast<A*>(this), src.begin(), src.end(), begin()
         );
      } catch (...) {
         deallocate(elems, capacity()); // <--
         throw;
      }
   }
   // HERE
   ~Vector() {
      destroy_with_allocator(*static_cast<A*>(this), begin(), end());
      deallocate(elems, capacity()); // <--
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
      construct(end(), val); // <--
      ++nelems;
   }
   void push_back(T&& val) {
      if (full())
         grow();
      construct(end(), std::move(val)); // <--
      ++nelems;
   }
   template <class ... Args>
   reference emplace_back(Args &&...args) {
      if (full())
         grow();
      construct(end(), std::forward<Args>(args)...);
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
      auto p = allocate(new_cap); // <--
      if constexpr (std::is_nothrow_move_assignable_v<T>) {
         // note: no try block
         uninitialized_move_with_allocator(
            *static_cast<A*>(this), begin(), end(), p
         );
      } else {
         auto src_p = begin();
         auto b = p, e = p + size();
         try {
            uninitialized_copy_with_allocator(
               *static_cast<A*>(this), begin(), end(), p
            );
         } catch (...) {
            deallocate(p, new_cap);
            throw;
         }
      }
      deallocate(elems, capacity());
      elems = p;
      cap = new_cap;
   }
   // HERE
   void resize(size_type new_cap) {
      if (new_cap <= capacity()) return;
      auto p = allocate(new_cap);
      if constexpr (std::is_nothrow_move_assignable_v<T>) {
         uninitialized_move_with_allocator(
            *static_cast<A*>(this), begin(), end(), p
         );
      } else {
         uninitialized_copy_with_allocator(
            *static_cast<A*>(this), begin(), end(), p
         );
      }
      try {
         uninitialized_fill_with_allocator(
            *static_cast<A*>(this), p + size(), p + new_cap, value_type{}
         );
         destroy_with_allocator(*static_cast<A*>(this), begin(), end());
         deallocate(elems, capacity());
         elems = p;
         nelems = cap = new_cap;
      } catch(...) {
         destroy_with_allocator(*static_cast<A*>(this), p, p + size());
         deallocate(p, new_cap);
         throw;
      }
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
//      if (std::cmp_less(remaining, n)) {
      if(cmp_less(remaining, n)) {
         auto index = std::distance(begin(), pos_);
         reserve(capacity() + n - remaining);
         pos_ = std::next(begin(), index);
      }

      const auto nb_to_uninit_displace =
         std::min<std::ptrdiff_t>(n, end() - pos_);
      auto where_to_uninit_displace = end() + n - nb_to_uninit_displace;
      if constexpr (std::is_nothrow_move_constructible_v<T>)
         uninitialized_move_with_allocator(
            *static_cast<A*>(this),
            end() - nb_to_uninit_displace, end(),
            where_to_uninit_displace
         );
      else
         uninitialized_copy_with_allocator(
            *static_cast<A*>(this),
            end() - nb_to_uninit_displace, end(),
            where_to_uninit_displace
         );

      // note : might be zero
      const auto nb_to_uninit_insert =
         std::max<std::ptrdiff_t>(0, n - nb_to_uninit_displace);
      auto where_to_uninit_insert = end();
      uninitialized_copy_with_allocator(
         *static_cast<A*>(this),
         last - nb_to_uninit_insert, last,
         where_to_uninit_insert
      );

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
      destroy(std::prev(end()));
      --nelems;
      return pos_;
   }
};


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

#include <iostream>
#include <vector>

template <class T, class A> // <--
std::ostream& operator<<(std::ostream& os, const Vector<T, A>& v) {
   if (v.empty()) return os;
   os << v.front();
   for (auto p = std::next(v.begin()); p != v.end(); ++p)
      os << ',' << *p;
   return os;
}

struct Data { int n; };

int main() {
   using namespace std::chrono;
   enum { N = 500'000 };
   {
      std::vector<Data> v;
      auto [r, dt] = test([](auto & v) {
         v.reserve(N);
         for(int i = 0; i != N; ++i)
            v.push_back({ i + 1 });
         return v.back();
      }, v);
      std::cout << "vector<Data>:\n\t"
                << v.size()
                << " insertions in "
                << duration_cast<microseconds>(dt).count()
                << " us\n";
   }
   {
      std::vector<Data, small_allocator<Data>> v;
      auto [r, dt] = test([](auto & v) {
         v.reserve(N);
         for(int i = 0; i != N; ++i)
            v.push_back({ i + 1 });
         return v.back();
      }, v);
      std::cout << "vector<Data, small_allocator<Data>>:\n\t"
                << v.size()
                << " insertions in "
                << duration_cast<microseconds>(dt).count()
                << " us\n";
   }
   {
      alignas(Data) char buf[N * sizeof(Data)];
      seq_buf_allocator<Data> alloc{ buf, sizeof buf };
      std::vector<Data, seq_buf_allocator<Data>> v(alloc);
      auto [r, dt] = test([](auto & v) {
         v.reserve(N);
         for(int i = 0; i != N; ++i)
            v.push_back({ i + 1 });
         return v.back();
      }, v);
      std::cout << "vector<Data, seq_buf_allocator<Data>>:\n\t"
                << v.size()
                << " insertions in "
                << duration_cast<microseconds>(dt).count()
                << " us\n";
   }
   {
      Vector<Data> v;
      auto [r, dt] = test([](auto & v) {
         v.reserve(N);
         for(int i = 0; i != N; ++i)
            v.push_back({ i + 1 });
         return v.back();
      }, v);
      std::cout << "Vector<Data>:\n\t"
                << v.size()
                << " insertions in "
                << duration_cast<microseconds>(dt).count()
                << " us\n";
   }
   {
      Vector<Data, small_allocator<Data>> v;
      auto [r, dt] = test([](auto & v) {
         v.reserve(N);
         for(int i = 0; i != N; ++i)
            v.push_back({ i + 1 });
         return v.back();
      }, v);
      std::cout << "Vector<Data, small_allocator<Data>>:\n\t"
                << v.size()
                << " insertions in "
                << duration_cast<microseconds>(dt).count()
                << " us\n";
   }
   {
      alignas(Data) char buf[N * sizeof(Data)];
      seq_buf_allocator<Data> alloc{ buf, sizeof buf };
      Vector<Data, seq_buf_allocator<Data>> v(alloc);
      auto [r, dt] = test([](auto & v) {
         v.reserve(N);
         for(int i = 0; i != N; ++i)
            v.push_back({ i + 1 });
         return v.back();
      }, v);
      std::cout << "Vector<Data, seq_buf_allocator<Data>>:\n\t"
                << v.size()
                << " insertions in "
                << duration_cast<microseconds>(dt).count()
                << " us\n";
   }
}
