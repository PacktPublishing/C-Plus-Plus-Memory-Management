// also available live: https://wandbox.org/permlink/WmsreHYVbbKuAYG0

#include <vector>
#include <memory>
#include <string>
#include <print>
#include <type_traits>

//
// use-case : destroy all GcNodes when GC object is destroyed, no finalization (restricted to trivially destructible types)
//
class GC {
   std::vector<void*> roots;
   GC() = default;
   static auto &get() {
      static GC gc;
      return gc;
   }
   template <class T, class ... Args>
      T *add_root(Args &&... args) {
         // there will be no finalization
         static_assert(std::is_trivially_destructible_v<T>);
         return static_cast<T*>(
            roots.emplace_back(
               new T(std::forward<Args>(args)...)
            )
         );
      }
   template <class T, class ... Args>
      friend T* gcnew(Args&&...);
public:
   ~GC() {
      std::print("~GC with {} objects to deallocate", std::size(roots));
      for(auto p : roots) std::free(p);
   }
   GC(const GC &) = delete;
   GC& operator=(const GC &) = delete;
};

template <class T, class ... Args>
   T *gcnew(Args &&...args) {
      return GC::get().add_root<T>(std::forward<Args>(args)...);
   }

// note: non trivially destructible
struct NamedThing {
   const char *name;
   NamedThing(const char *name) : name{ name } {
      std::print("{} ctor\n", name);
   }
   ~NamedThing() {
      std::print("{} dtor\n", name);
   }
};

struct Identifier {
   int value;
};

// would not compile
/*
void g() {
   [[maybe_unused]] auto p = gcnew<NamedThing>("hi");
   [[maybe_unused]] auto q = gcnew<NamedThing>("there");
}
*/

void g() {
   [[maybe_unused]] auto p = gcnew<Identifier>(2);
   [[maybe_unused]] auto q = gcnew<Identifier>(3);
}

auto h() {
   struct X {
      int m() const { return 123; }
   };
   return gcnew<X>();
}

auto f() {
   g();
   return h();
}

int main() {
   std::print("Pre\n");
   std::print("{}\n", f()->m());
   std::print("Post\n");
}
