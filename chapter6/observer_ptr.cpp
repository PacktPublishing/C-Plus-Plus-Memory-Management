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
      constexpr bool operator==(const observer_ptr &autre) const {
         return p == autre.p;
      }
      constexpr bool operator!=(const observer_ptr &autre) const {
         return !(*this == autre);
      }
      template <class U>
         constexpr bool operator==(const observer_ptr<U> &autre) const {
            return p == &*autre;
         }
      template <class U>
         constexpr bool operator!=(const observer_ptr<U> &autre) const {
            return !(*this == autre);
         }
      template <class U>
         constexpr bool operator==(const U *q) const {
            return p == q;
         }
      template <class U>
         constexpr bool operator!=(const U *q) const {
            return !(*this == q);
         }
      void swap(observer_ptr &autre) {
         using std::swap;
         swap(p, autre.p);
      }
      constexpr operator bool() const noexcept {
         return p != nullptr;
      }
      observer_ptr(observer_ptr &&autre) : p{ autre.p } {
         autre.p = nullptr;
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