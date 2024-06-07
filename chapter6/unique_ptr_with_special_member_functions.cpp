// also available live: https://wandbox.org/permlink/4lHPJyutAfEZMyvn

#include <iostream>
#include <type_traits>
#include <utility>

namespace managing_memory_book {
   // basic deleter types
   template <class T>
   struct deleter_pointer_wrapper {
      void (*pf)(T*);
      deleter_pointer_wrapper(void (*pf)(T*)) : pf{ pf } {
      }
      void operator()(T* p) const {
         std::cout << "pf(p)\n";
         pf(p);
      }
   };
   template <class T>
   struct default_deleter {
      void operator()(T* p) const {
         std::cout << "delete p\n";
         delete p;
      }
   };
   template <class T>
   struct default_deleter<T[]> {
      void operator()(T* p) const {
         std::cout << "delete [] p\n";
         delete[] p;
      }
   };
   template <class T>
   struct is_deleter_function_candidate : std::false_type {}
   ;
   template <class T>
   struct is_deleter_function_candidate<void (*)(T*)> : std::true_type {}
   ;
   template <class T>
   constexpr auto is_deleter_function_candidate_v =
      is_deleter_function_candidate<T>::value;
   // unique_ptr general template
   template <class T, class D = default_deleter<T>>
   class unique_ptr : std::conditional_t <
      is_deleter_function_candidate_v<D>,
      deleter_pointer_wrapper<T>,
      D
   > {
      using deleter_type = std::conditional_t <
         is_deleter_function_candidate_v<D>,
         deleter_pointer_wrapper<T>,
         D
      >;
      T* p = nullptr;
   public:
      unique_ptr() = default;
      unique_ptr(T* p) : p{ p } {
         std::cout << "general template\n";
      }
      unique_ptr(T* p, void (*pf)(T*))
         : deleter_type{ pf }, p{ p } {
         std::cout << "specialization for function pointer\n";
      }
      ~unique_ptr() {
         (*static_cast<deleter_type*>(this))(p);
      }
      unique_ptr(const unique_ptr&) = delete;
      unique_ptr& operator=(const unique_ptr&) = delete;
      void swap(unique_ptr &other) noexcept {
         using std::swap;
         swap(p, other.p);
      }
      unique_ptr(unique_ptr &&other) noexcept
         : p{ std::exchange(other.p, nullptr) } {
      }
      unique_ptr& operator=(unique_ptr &&other) noexcept {
         unique_ptr{ std::move(other) }.swap(*this);
         return *this;
      }
   };
   // unique_ptr specialization for arrays
   template <class T, class D>
   class unique_ptr<T[], D> : std::conditional_t <
      is_deleter_function_candidate_v<D>,
      deleter_pointer_wrapper<T>,
      D
   > {
      using deleter_type = std::conditional_t <
         is_deleter_function_candidate_v<D>,
         deleter_pointer_wrapper<T>,
         D
      >;
      T* p = nullptr;
   public:
      unique_ptr() = default;
      unique_ptr(T* p) : p{ p } {
         std::cout << "specialization for arrays\n";
      }
      unique_ptr(T* p, void (*pf)(T*))
         : deleter_type{ pf }, p{ p } {
         std::cout << "specialization for arrays with function pointer\n";
      }
      ~unique_ptr() {
         (*static_cast<deleter_type*>(this))(p);
      }
      unique_ptr(const unique_ptr&) = delete;
      unique_ptr& operator=(const unique_ptr&) = delete;
      void swap(unique_ptr &other) noexcept {
         using std::swap;
         swap(p, other.p);
      }
      unique_ptr(unique_ptr &&other) noexcept
         : p{ std::exchange(other.p, nullptr) } {
      }
      unique_ptr& operator=(unique_ptr &&other) noexcept {
         unique_ptr{ std::move(other) }.swap(*this);
         return *this;
      }
   };
}

void end_of_life(int*) {}
int main() {
   using managing_memory_book::unique_ptr;

   [[maybe_unused]] unique_ptr<int> p0{ new int{ 3 } };
   [[maybe_unused]] unique_ptr<int[]> p1{ new int[10] };
   [[maybe_unused]] unique_ptr<int, void (*)(int*)> p2{ new int{ 4 }, end_of_life };
}