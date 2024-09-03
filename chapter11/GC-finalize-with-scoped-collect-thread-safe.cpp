// also available live: https://wandbox.org/permlink/TX2CQ4IanSIVYAY5

#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <atomic>
#include <functional>
#include <utility>
#include <mutex>

//
// use-case : destroy all GcNodes when collecting, calling the proper dtors
//
// using counting_ptr to know which objects to collect
//
template <class T>
   class counting_ptr {
      using count_type = std::atomic<int>;
      T *p;
      count_type *count;
      std::function<void()> mark;
   public:
      template <class M>
         constexpr counting_ptr(T *p, M mark) try : p{ p }, mark{ mark } {
            count = new count_type{ 1 };
         } catch(...) {
            delete p;
            throw;
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
      constexpr bool operator==(const counting_ptr &other) const {
         return p == other.p;
      }
      constexpr bool operator!=(const counting_ptr &other) const {
         return !(*this == other);
      }
      template <class U>
         constexpr bool operator==(const counting_ptr<U> &other) const {
            return p == &*other;
         }
      template <class U>
         constexpr bool operator!=(const counting_ptr<U> &other) const {
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
      void swap(counting_ptr &other) {
         using std::swap;
         swap(p, other.p);
         swap(count, other.count);
         swap(mark, other.mark);
      }
      constexpr operator bool() const noexcept {
         return p != nullptr;
      }
      counting_ptr(counting_ptr &&other) noexcept
         : p{ std::exchange(other.p, nullptr) },
           count{ std::exchange(other.count, nullptr) },
           mark{ other.mark } {
      }
      counting_ptr &operator=(counting_ptr &&other) noexcept {
         counting_ptr{ std::move(other) }.swap(*this);
         return *this;
      }
      counting_ptr(const counting_ptr &other)
         : p{ other.p }, count{ other.count }, mark{ other.mark } {
         if (count) ++(*count);
      }
      counting_ptr &operator=(const counting_ptr &other) {
         counting_ptr{ other }.swap(*this);
         return *this;
      }
      ~counting_ptr() {
         if (count) {
            if ((*count)-- == 1) {
               mark();
               delete count;
            }
         }
      }
   };
namespace std {
   template <class T, class M>
      void swap(counting_ptr<T> &a, counting_ptr<T> &b) {
         a.swap(b);
      }
}

class GC {
   std::mutex m;
   class GcRoot {
      void *p;
   public:
      auto get() const noexcept { return p; }
      GcRoot(void *p) : p{ p } {
      }
      GcRoot(const GcRoot&) = delete;
      GcRoot& operator=(const GcRoot&) = delete;
      virtual void destroy(void*) const noexcept = 0;
      virtual ~GcRoot() = default;
   };
   template <class T> class GcNode : public GcRoot {
      void destroy(void *q) const noexcept override {
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
   std::vector<std::pair<std::unique_ptr<GcRoot>, bool>> roots;
   GC() = default;
   static auto &get() {
      static GC gc;
      return gc;
   }
   void make_collectable(void *p) {
      std::lock_guard _ { m };
      for (auto &[q, coll] : roots)
         if (static_cast<GcRoot*>(p) == q.get())
            coll = true;
   }
   void collect() {
      std::lock_guard _ { m };
      for (auto p = std::begin(roots); p != std::end(roots); ) {
         if (auto &[ptr, collectible] = *p; collectible) {
            ptr = nullptr;
            p = roots.erase(p);
         } else {
            ++p;
         }

      }
   }
   template <class T, class ... Args>
      auto add_root(Args &&... args) {
         std::lock_guard _ { m };
         auto q = static_cast<T*>(roots.emplace_back(
            std::make_unique<GcNode<T>>(std::forward<Args>(args)...), false
         ).first->get());
         return counting_ptr{
            q, [&,q]() {
               std::lock_guard _ { m };
               for (auto &[p, coll] : roots)
                  if (static_cast<void*>(q) == p.get()->get()) {
                     coll = true;
                     return;
                  }
            }
         };
      }
   template <class T, class ... Args>
      friend counting_ptr<T> gcnew(Args&&...);
   friend struct scoped_collect;
public:
   GC(const GC &) = delete;
   GC& operator=(const GC &) = delete;
};

struct scoped_collect {
   scoped_collect() = default;
   scoped_collect(const scoped_collect &) = delete;
   scoped_collect(scoped_collect &&) = delete;
   scoped_collect& operator=(const scoped_collect &) = delete;
   scoped_collect &operator=(scoped_collect &&) = delete;
   ~scoped_collect() {
      GC::get().collect();
   }
};


template <class T, class ... Args>
   counting_ptr<T> gcnew(Args &&... args) {
      return GC::get().add_root<T>(std::forward<Args>(args)...);
   }

struct NamedThing {
   const char *name;
   NamedThing(const char *name) : name{ name } {
      std::cout << name << " ctor" << std::endl;
   }
   ~NamedThing() {
      std::cout << name << " dtor" << std::endl;
   }
};

auto g() {
   auto _ = scoped_collect{};
   [[maybe_unused]] auto p = gcnew<NamedThing>("hi");
   auto q = gcnew<NamedThing>("there");
   return q;
}

auto h() {
   struct X {
      int m() const { return 123; }
   };
   return gcnew<X>();
}

auto f() {
   auto _ = scoped_collect{};
   auto p = g();
   std::cout << '\"' << p->name << '\"' << std::endl;
}

int main() {
   using namespace std;
   cout << "Pre" << endl;
   f();
   cout << h()->m() << endl;
   cout << "Post" << endl;
}
