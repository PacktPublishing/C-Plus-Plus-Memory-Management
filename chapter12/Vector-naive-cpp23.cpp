// also available live: https://wandbox.org/permlink/AuGb4nRHnH8BQHFo

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

template <class T>
   std::ostream& operator<<(std::ostream &os, const Vector<T> &v) {
      if(v.empty()) return os;
      os << v.front();
      for(auto p = std::next(v.begin()); p != v.end(); ++p)
         os << ',' << *p;
      return os;
   }

int main() {
   Vector v0{ 2,3,5,7,11 };
   Vector v1 = v0; // copy ctor
   std::cout << v1 << '\n'; // 2,3,5,7,11
   for(int n : { 13, 17, 19, 23, 29, 31, 37, 41, 43, 47 })
      v0.push_back(n); // will call grow() at some point
   // Size: 15, capacity: 20
   // 2,3,5,7,11,13,17,19,23,29,31,37,41,43,47
   std::cout << "Size: " << v0.size() << ", capacity: " << v0.capacity() << '\n'
             << v0 << '\n';
}
