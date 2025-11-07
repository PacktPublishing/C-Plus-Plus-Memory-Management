#ifndef OBSERVER_PTR_H
#define OBSERVER_PTR_H
#include <utility>
#include <cassert>
template <class T>
   class observer_ptr {
      T *p;
   public:
      constexpr observer_ptr(T *p) noexcept : p{ p } {
      }
      T& operator*() noexcept {
         return *p;
      }
      const T& operator*() const noexcept {
         return *p;
      }
      T* operator->() noexcept {
         return p;
      }
      const T* operator->() const noexcept {
         return p;
      }
      constexpr bool operator==(const observer_ptr &other) const {
         return p == other.p;
      }
      constexpr bool operator!=(const observer_ptr &other) const {
         return !(*this == other);
      }
      template <class U>
         constexpr bool operator==(const observer_ptr<U> &other) const {
            return p == &*other;
         }
      template <class U>
         constexpr bool operator!=(const observer_ptr<U> &other) const {
            return !(*this == other);
         }
      template <class U>
         constexpr bool operator==(const U *q) const {
            return p == q;
         }
      template <class U>
         constexpr bool operator!=(const U *q) const {
            return !(*this == q);
         }
      void swap(observer_ptr &other) {
         using std::swap;
         swap(p, other.p);
      }
      constexpr operator bool() const noexcept {
         return p != nullptr;
      }
      observer_ptr(observer_ptr &&other) : p{ other.p } {
         other.p = nullptr;
      }
      observer_ptr& operator=(observer_ptr &&other) { // unsafe for swap(a,b)
         assert(this != &other);
         p = other.p;
         other.p = nullptr;
         return *this;
      }
      observer_ptr(const observer_ptr&) = default;
      observer_ptr& operator=(const observer_ptr&) = default;
      ~observer_ptr() = default;
   };
namespace std {
   template <class T> void swap(observer_ptr<T> &a, observer_ptr<T> &b) {
      a.swap(b);
   }
}

#endif
