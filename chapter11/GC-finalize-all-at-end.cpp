// also available live: https://wandbox.org/permlink/yVHyuCA3sB4ByMWZ

#include <vector>
#include <memory>
#include <print>

//
// use-case : destroy all GcNodes when GC object is destroyed, calling the proper dtors
//
class GC {
   class GcRoot {
      void *p;
   public:
      auto get() const noexcept { return p; }
      GcRoot(void *p) : p{ p } {
      }
      GcRoot(const GcRoot &) = delete;
      GcRoot& operator=(const GcRoot &) = delete;
      virtual void destroy(void *) const noexcept = 0;
      virtual ~GcRoot() = default;
   };
   template <class T> class GcNode : public GcRoot {
      void destroy(void* q) const noexcept override {
         delete static_cast<T*>(q);
      }
   public:
      template <class ... Args>
         GcNode(Args &&... args) : GcRoot(new T(std::forward<Args>(args)...)) {
         }
      ~GcNode() {
         destroy(get());
      }
   };
   std::vector<std::unique_ptr<GcRoot>> roots;
   GC() = default;
   static auto &get() {
      static GC gc;
      return gc;
   }
   template <class T, class ... Args>
      T *add_root(Args &&... args) {
         return static_cast<T*>(roots.emplace_back(
            std::make_unique<GcNode<T>>(std::forward<Args>(args)...)
         )->get());
      }
   template <class T, class ... Args>
      friend T* gcnew(Args&&...);
public:
   GC(const GC &) = delete;
   GC& operator=(const GC &) = delete;
};

template <class T, class ... Args>
   T *gcnew(Args &&...args) {
      return GC::get().add_root<T>(std::forward<Args>(args)...);
   }

struct NamedThing {
   const char *name;
   NamedThing(const char *name) : name{ name } {
      std::print("{} ctor\n", name);
   }
   ~NamedThing() {
      std::print("{} dtor\n", name);
   }
};

void g() {
   [[maybe_unused]] auto p = gcnew<NamedThing>("hi");
   [[maybe_unused]] auto q = gcnew<NamedThing>("there");
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
