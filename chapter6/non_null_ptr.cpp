class invalid_pointer {};
template <class T>
   class non_null_ptr {
      T *p;
   public:
      non_null_ptr(T *p) : p{ p } {
         if (!p) throw invalid_pointer{};
      }
      constexpr operator bool() const noexcept {
         return true;
      }
      T& operator*() noexcept { return *p; }
      const T& operator*() const noexcept { return *p; }
      T* operator->() noexcept {
         return p;
      }
      const T* operator->() const noexcept {
         return p;
      }
      bool operator==(const non_null_ptr &other) const {
         return p == other.p;
      }
      bool operator!=(const non_null_ptr &other) const {
         return !(*this == other);
      }
      // etc.
   };
   