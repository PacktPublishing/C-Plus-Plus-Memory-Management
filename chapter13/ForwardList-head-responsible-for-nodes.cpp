// also available live: https://wandbox.org/permlink/vQczKJu2kH2CTXcZ

#include <cstddef>
#include <algorithm>
#include <utility>
#include <iterator>
#include <initializer_list>
#include <concepts>
#include <memory> // <--

template <class T>
class ForwardList {
public:
   using value_type = T;
   using size_type = std::size_t;
   using pointer = T*;
   using const_pointer = const T*;
   using reference = T&;
   using const_reference = const T&;
private:
   struct Node {
      value_type value;
      Node *next = nullptr;
      Node(const_reference value) : value { value } {
      }
      Node(value_type &&value) : value { std::move(value) } {
      }
   };
   struct deleter { // <--
      void operator()(Node *p) const {
         while(p) {
            Node *q = p->next;
            delete p;
            p = q;
         }
      }
   };
   std::unique_ptr<Node, deleter> head;
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
      friend class ForwardList<U>;
   };
public:
   using iterator = Iterator<T>;
   using const_iterator = Iterator<const T>;
   iterator begin() { return { head.get() }; } // <--
   const_iterator begin() const { return { head.get() }; } // <--
   const_iterator cbegin() const { return begin(); }
   iterator end() { return {}; }
   const_iterator end() const { return {}; }
   const_iterator cend() const { return end(); }
   void clear() noexcept {
      for(auto p = head.get(); p; ) { // <--
         auto q = p->next;
         delete p;
         p = q;
      }
      nelems = 0;
   }
   // ...
   ForwardList() = default;
   template <std::forward_iterator It>
      ForwardList(It b, It e) {
         if(b == e) return;
         head.reset(new Node{ *b }); // <--
         auto q = head.get(); // <--
         ++nelems;
         for(++b; b != e; ++b) {
            q->next = new Node{ *b };
            q = q->next;
            ++nelems;
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
   ~ForwardList() = default;
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
      auto p = new Node{ val };
      p->next = head.get(); // <--
      head.release(); // <--
      head.reset(p); // <--
      ++nelems;
   }
   void push_front(T&& val) {
      auto p = new Node{ std::move(val) };
      p->next = head.get(); // <--
      head.release(); // <--
      head.reset(p); // <--
      ++nelems;
   }
   // ...
   iterator insert_after(iterator pos, const_reference value) {
      auto p = new Node{ value };
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

template <class T>
std::ostream& operator<<(std::ostream& os, const ForwardList<T>& lst) {
   if (lst.empty()) return os;
   os << lst.front();
   for (auto p = std::next(lst.begin()); p != lst.end(); ++p)
      os << ',' << *p;
   return os;
}

int main() {
   ForwardList lst0{ 2,3,5,7,11 };
   ForwardList lst1 = lst0; // copy ctor
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
