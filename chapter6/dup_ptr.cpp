#ifndef DUP_PTR_H
#define DUP_PTR_H

#include <type_traits>
#include <concepts>
#include <utility>

namespace growing_smart_pointer {
   //
   // default duplicating functors
   //
   struct Copier {
      // precondition : p != nullptr
      template <class T> T* operator()(const T *p) const {
         return new T{ *p };
      }
   };
   struct Cloner {
      // precondition : p != nullptr
      template <class T> T* operator()(const T *p) const {
         return p->clone();
      }
   };
   namespace v0 {
      //
      //
      // hypothetical «cloneable» interface
      struct cloneable {
         virtual cloneable *clone() const = 0;
         virtual ~cloneable() = default;
      protected:
         cloneable() = default;
         cloneable(const cloneable&) = default;
      };

      //
      //
      //
      template <class T, class Dup = std::conditional_t<
                  std::is_base_of_v<cloneable, T>, Cloner, Copier
               >>
         class dup_ptr {
            T *p{};
         public:
            dup_ptr() = default;
            constexpr bool empty() const { return !p; }
            constexpr operator bool() const { return !empty(); }
            dup_ptr(T *p) noexcept : p{ p } {
            }
            ~dup_ptr() {
               delete p;
            }
            dup_ptr(dup_ptr &&other)
               : p{ std::exchange(other.p, nullptr) } {
            }
            dup_ptr& operator=(dup_ptr &&other) {
               dup_ptr{ std::move(other) }.swap(*this);
               return *this;
            }
            T& operator*() noexcept { return *p; }
            const T& operator*() const noexcept { return *p; }
            T* operator->() noexcept { return p; }
            const T* operator->() const noexcept { return p; }
            // ... use a Dup object whenever we want to duplicate *p
            void swap(dup_ptr &other) {
               using std::swap;
               swap(p, other.p);
            }
            dup_ptr(const dup_ptr &other)
               : p{ other.empty()? nullptr : Dup{}(other.p) } {
            }
            dup_ptr& operator=(const dup_ptr &other) {
               dup_ptr{ other }.swap(*this);
               return *this;
            }
         };
   }
   namespace v1 {
      //
      //
      //
      template <class, class = void>
         struct has_clone : std::false_type { };
      template <class T>
         struct has_clone <T, std::void_t<
               decltype(std::declval<const T*>()->clone())
            >>
            : std::true_type {
         };
      template <class T>
         constexpr bool has_clone_v = has_clone<T>::value;
      //
      //
      //
      template <class T, class Dup = std::conditional_t<
                   has_clone_v<T>, Cloner, Copier
               >>
         class dup_ptr {
            T *p{};
         public:
            dup_ptr() = default;
            constexpr bool empty() const { return !p; }
            constexpr operator bool() const { return !empty(); }
            dup_ptr(T *p) noexcept : p{ p } {
            }
            ~dup_ptr() {
               delete p;
            }
            dup_ptr(dup_ptr &&other)
               : p{ std::exchange(other.p, nullptr) } {
            }
            dup_ptr& operator=(dup_ptr &&other) {
               dup_ptr{ std::move(other) }.swap(*this);
               return *this;
            }
            T& operator*() noexcept { return *p; }
            const T& operator*() const noexcept { return *p; }
            T* operator->() noexcept { return p; }
            const T* operator->() const noexcept { return p; }
            // ... use a Dup object whenever we want to duplicate *p
            void swap(dup_ptr &other) {
               using std::swap;
               swap(p, other.p);
            }
            dup_ptr(const dup_ptr &other)
               : p{ other.empty()? nullptr : Dup{}(other.p) } {
            }
            dup_ptr& operator=(const dup_ptr &other) {
               dup_ptr{ other }.swap(*this);
               return *this;
            }
         };
   }
   namespace v2 {
      //
      //
      //
      template <class T>
         concept cloneable = requires(const T *p) {
            { p->clone() } -> std::convertible_to<T*>;
         };
      //
      //
      //
      template <class T, class Dup = std::conditional_t<
                   cloneable<T>, Cloner, Copier
               >>
         class dup_ptr {
            T *p{};
         public:
            dup_ptr() = default;
            constexpr bool empty() const { return !p; }
            constexpr operator bool() const { return !empty(); }
            dup_ptr(T *p) noexcept : p{ p } {
            }
            ~dup_ptr() {
               delete p;
            }
            dup_ptr(dup_ptr &&other)
               : p{ std::exchange(other.p, nullptr) } {
            }
            dup_ptr& operator=(dup_ptr &&other) {
               dup_ptr{ std::move(other) }.swap(*this);
               return *this;
            }
            T& operator*() noexcept { return *p; }
            const T& operator*() const noexcept { return *p; }
            T* operator->() noexcept { return p; }
            const T* operator->() const noexcept { return p; }
            // ... use a Dup object whenever we want to duplicate *p
            void swap(dup_ptr &other) {
               using std::swap;
               swap(p, other.p);
            }
            dup_ptr(const dup_ptr &other)
               : p{ other.empty()? nullptr : Dup{}(other.p) } {
            }
            dup_ptr& operator=(const dup_ptr &other) {
               dup_ptr{ other }.swap(*this);
               return *this;
            }
         };
   }
}

#endif