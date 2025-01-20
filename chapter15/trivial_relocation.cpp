//
// ... includes and other tangentially relevant stuff ...
//

namespace without_relocation {
//
// This is not a good function interface, but we want to
// keep the example relatively simple
//
template <class T>
   void resize(T *&arr, std::size_t old_cap, std::size_t new_cap) {
      //
      // we could deal with throwing a default constructor
      // but it would complicate our code a bit and these
      // added complexities, worthwhile as they are, are
      // besides the point for what we are discussing here
      //
      static_assert(
         std::is_nothrow_default_contructible_v<T>
      );
      //
      // sometimes, there's just nothing to do
      //
      if(new_cap <= old_cap) return arr;
      //
      // allocate a chunk of raw memory (no object created)
      //
      auto p = static_cast<T*>(
         std::malloc(new_cap * sizeof(T))
      );
      if(!p) throw std::bad_alloc{};
      //
      // if move assignment does not throw, be aggressive
      //
      if constexpr(std::is_nothrow_move_assignable_v<T>) {
         std::uninitialized_move(arr, arr + old_cap, p);
         std::destroy(arr, arr + old_cap);
      } else {
         //
         // since move assignment could throw, let's be
         // conservative and copy instead
         //
         try {
            std::uninitialized_copy(arr, arr + old_cap, p);
            std::destroy(arr, arr + old_cap);
         } catch (...) {
            std::free(p);
            throw;
         }
      }
      //
      // fill the remaining space with default objects
      // (remember: we statically asserted that T::T() is
      // non-throwing)
      //
      std::uninitialized_default_construct(
         p + old_cap, p + new_cap
      );
      //
      // replace the old memory block (now without objects)
      // with the new one
      //
      std::free(arr);
      arr = p;
   }
}

/////////////////////

namespace with_relocation {
//
// This is not a good function interface, but we want to
// keep the example relatively simple
//
template <class T>
   void resize(T * &arr, std::size_t old_cap, std::size_t new_cap) {
      //
      // we could deal with throwing a default constructor
      // but it would complicate our code a bit and these
      // added complexities, worthwhile as they are, are
      // besides the point for what we are discussing here
      //
      static_assert(
         std::is_nothrow_default_contructible_v<T>
      );
      //
      // sometimes, there's just nothing to do
      //
      if(new_cap <= old_cap) return arr;
      //
      // allocate a chunk of raw memory (no object created)
      //
      auto p = static_cast<T*>(
         std::malloc(new_cap * sizeof(T))
      );
      if(!p) throw std::bad_alloc{};
      //
      // this is our ideal case
      //
      if constexpr (std::is_trivially_relocatable_v<T>) {
         // equivalent to memcpy() plus consider the
         // lifetime of objects in [arr, arr + old_cap)
         // finished and the lifetime of objects in
         // [p, p + old_cap) started
         //
         // note: this supposes that the trait
         // std::is_trivially_relocatable<T>
         // implies std::is_trivially_destructible<T>
         std::relocate(arr, arr + old_cap, p);
      //
      // if move assignment does not throw, be aggressive
      //
      } else if constexpr(std::is_nothrow_move_assignable_v<T>){
         std::uninitialized_move(arr, arr + old_cap, p);
         std::destroy(arr, arr + old_cap);
      } else {
         //
         // since move assignment could throw, let's be
         // conservative and copy instead
         //
         try {
            std::uninitialized_copy(arr, arr + old_cap, p);
            std::destroy(arr, arr + old_cap);
         } catch (...) {
            std::free(p);
            throw;
         }
      }
      //
      // fill the remaining space with default objects
      // (remember: we statically asserted that T::T() is
      // non-throwing)
      //
      std::uninitialized_default_construct(
         p + old_cap, p + new_cap
      );
      //
      // replace the old memory block (now without objects)
      // with the new one
      //
      std::free(arr);
      arr = p;
   }
}