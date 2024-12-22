// also available live: https://wandbox.org/permlink/obuBtRwDEqq79HgE

#include <cstddef>
#include <algorithm>
#include <utility>
#include <initializer_list>
#include <iterator>
#include <cstdlib>
#include <memory>
#include <limits>

template <class T>
struct small_allocator {
   using value_type = T;
   T* allocate(std::size_t n) {
      auto p = static_cast<T*>(malloc(n * sizeof(value_type)));
      if (!p) throw std::bad_alloc{};
      return p;
   }
   void deallocate(T *p, std::size_t) {
       free(p);
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

template <class T, class A = std::allocator<T>>
class ForwardList {
public:
   // note: these are the forward-facing types, expressed
   // in terms where T is the value_type
   using value_type = T;
   using size_type = typename std::allocator_traits<A>::size_type;
   using pointer = value_type*;
   using const_pointer = const value_type*;
   using reference = value_type&;
   using const_reference = const value_type&;
private:
   struct Node {
      value_type value;
      Node *next = nullptr;
      Node(const_reference value) : value { value } {
      }
      Node(value_type &&value) : value { std::move(value) } {
      }
   };
   using Alloc = typename std::allocator_traits<A>::template rebind_alloc<Node>;
   Alloc alloc; // <--
   Node *head {};
   size_type nelems {};
   // ...
public:
   size_type size() const { return nelems; }
   bool empty() const { return !head; }
private:
   template <class U> class Iterator {
   public:
      using value_type = typename ForwardList<T>::value_type;
      using pointer = typename ForwardList<T>::pointer;
      using reference = typename ForwardList<T>::reference;
      using difference_type = std::ptrdiff_t;
      using iterator_category = std::forward_iterator_tag;
      friend class Iterator<T>;
      friend class Iterator<const T>;
   private:
      Node *cur {};
      friend class ForwardList<T,A>; // <--
   public:
      Iterator() = default;
      Iterator(Node *p) : cur { p } {
      }
      Iterator& operator++() {
         cur = cur->next;
         return *this;
      }
      Iterator operator++(int) {
         auto temp = *this;
         operator++();
         return temp;
      }
      bool operator==(const Iterator &other) const {
         return cur == other.cur;
      }
      // not needed since C++20
      bool operator!=(const Iterator &other) const {
         return !(*this == other);
      }
      U& operator*() { return cur->value; }
      const U& operator*() const { return cur->value; }
      U* operator->() { return cur->value; }
      const U* operator->() const { return cur->value; }
   };
public:
   using iterator = Iterator<T>;
   using const_iterator = Iterator<const T>;
   iterator begin() { return { head }; }
   const_iterator begin() const { return { head }; }
   const_iterator cbegin() const { return begin(); }
   iterator end() { return {}; }
   const_iterator end() const { return {}; }
   const_iterator cend() const { return end(); }
   void clear() noexcept {
      for(auto p = head; p; ) {
         auto q = p->next;
         std::allocator_traits<Alloc>::destroy(alloc, p); // <--
         std::allocator_traits<Alloc>::deallocate(alloc, p, 1); // <--
         p = q;
      }
      nelems = 0;
   }
   // ...
   ForwardList() = default;
   template <std::forward_iterator It>
      ForwardList(It b, It e) {
         if(b == e) return;
         try {
            head = std::allocator_traits<Alloc>::allocate(alloc, 1); // <--
            std::allocator_traits<Alloc>::construct(alloc, head, *b); // <--
            auto q = head;
            ++nelems;
            for(++b; b != e; ++b) {
               auto ptr = std::allocator_traits<Alloc>::allocate(alloc, 1); // <--
               std::allocator_traits<Alloc>::construct(alloc, ptr, *b); // <--
               q->next = ptr; // <--
               q = q->next;
               ++nelems;
            }
         } catch (...) {
            clear();
            throw;
         }
      }
   ForwardList(const ForwardList& other)
      : ForwardList(other.begin(), other.end()) {
   }
   ForwardList(std::initializer_list<T> other)
      : ForwardList(other.begin(), other.end()) {
   }
   ForwardList(ForwardList&& other) noexcept
      : head{ std::exchange(other.head, nullptr) },
        nelems{ std::exchange(other.nelems, 0) } {
   }
   // ...
   // ...
   ~ForwardList() {
      clear();
   }
   // ...
   void swap(ForwardList& other) noexcept {
      using std::swap;
      swap(head, other.head);
      swap(nelems, other.nelems);
   }
   ForwardList& operator=(const ForwardList& other) {
      ForwardList{ other }.swap(*this);
      return *this;
   }
   ForwardList& operator=(ForwardList&& other) {
      ForwardList{ std::move(other) }.swap(*this);
      return *this;
   }
   // ...
   reference front() { return head->value; }
   const_reference front() const { return head->value; }
   bool operator==(const ForwardList &other) const {
      return size() == other.size() &&
             std::equal(begin(), end(), other.begin());
   }
   // can be omitted since C++20
   bool operator!=(const ForwardList &other) const {
      return !(*this == other);
   }
   // ...
   void push_front(const_reference val) {
      auto p = std::allocator_traits<Alloc>::allocate(alloc, 1); // <--
      std::allocator_traits<Alloc>::construct(alloc, p, val); // <--
      p->next = head;
      head = p;
      ++nelems;
   }
   void push_front(T&& val) {
      auto p = std::allocator_traits<Alloc>::allocate(alloc, 1); // <--
      std::allocator_traits<Alloc>::construct(alloc, p, std::move(val)); // <--
      p->next = head;
      head = p;
      ++nelems;
   }
   // ...
   iterator insert_after(iterator pos, const_reference value) {
      auto p = std::allocator_traits<Alloc>::allocate(alloc, 1); // <--
      std::allocator_traits<Alloc>::construct(alloc, p, value); // <--
      p->next = pos.cur->next;
      pos.cur->next = p;
      ++nelems;
      return { p };
   }
   template <std::input_iterator It>
      iterator insert_after(iterator pos, It b, It e) {
         for(; b != e; ++b)
            pos = insert_after(pos, *b);
         return pos;
      }
};

#include <iostream>

template <class T, class A>
std::ostream& operator<<(std::ostream& os, const ForwardList<T, A>& lst) {
   if (lst.empty()) return os;
   os << lst.front();
   for (auto p = std::next(lst.begin()); p != lst.end(); ++p)
      os << ',' << *p;
   return os;
}

template <template <class> class A>
   void tests() {
      ForwardList<int, A<int>> lst0{ 2,3,5,7,11 };
      ForwardList<int, A<int>> lst1 = lst0; // copy ctor
      std::cout << lst1 << '\n'; // 2,3,5,7,11
      for (int n : { 13, 17, 19, 23, 29, 31, 37, 41, 43, 47 })
         lst0.push_front(n);
      // Size: 15
      // 47,43,41,37,31,29,23,18,17,13,2,3,5,7,11
      std::cout << "Size: " << lst0.size() << '\n'
                << lst0 << '\n';
      // Size: 5
      // 2,3,5,7,11
      std::cout << "Size: " << lst1.size() << '\n'
                << lst1 << '\n';
      auto pos = lst1.insert_after(lst1.begin(), -1);
      // Size: 6
      // 2,-1,3,5,7,11
      std::cout << "Size: " << lst1.size() << '\n'
                << lst1 << '\n';
      int arr[]{ -2, -3, -4 };
      lst1.insert_after(pos, std::begin(arr), std::end(arr));
      // Size: 9
      // 2,-1,-2,-3,-4,3,5,7,11
      std::cout << "Size: " << lst1.size() << '\n'
                << lst1 << '\n';
   }

int main() {
   tests<std::allocator>();
   std::cout << "-=-=-=-=-=-=-=-\n";
   tests<small_allocator>();
}
