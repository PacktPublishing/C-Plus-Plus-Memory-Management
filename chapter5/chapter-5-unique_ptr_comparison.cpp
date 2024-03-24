// also available live: https://wandbox.org/permlink/YmBbYjWRfu4LXsvz

#include <memory>
template <class T>
   bool point_to_same(const std::unique_ptr<T> &p0, const std::unique_ptr<T> &p1) {
      return p0 == p1;
   }
template <class T>
   bool have_same_value(const std::unique_ptr<T> &p0, const std::unique_ptr<T> &p1) {
      return p0 && p1 && *p0 == *p1;
   }
#include <cassert>
int main() {
   // two distinct pointers to objects with same value
   std::unique_ptr<int> a{ new int { 3 } };
   std::unique_ptr<int> b{ new int { 3 } };
   assert(point_to_same(a, a) && have_same_value(a, a));
   assert(!point_to_same(a, b) && have_same_value(a, b));
}
