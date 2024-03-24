// also available live: https://wandbox.org/permlink/UOKnwrQvMTdilfuA

#include <memory>
class requires_dynamic_alloc {
   ~requires_dynamic_alloc() = default; // private
   // ...
   friend struct cleaner;
};
// ...
struct cleaner {
   template <class T>
      void operator()(T *p) const { delete p; }
};
int main() {
   using namespace std;
   // requires_dynamic_alloc r0; // no
   //auto p0 = unique_ptr<requires_dynamic_alloc>{
   //   new requires_dynamic_alloc
   //}; // no, as delete not available to default deleter
   auto p1 = unique_ptr<requires_dynamic_alloc, cleaner>{
      new requires_dynamic_alloc
   }; // ok, will use cleaner::operator() to delete pointee
}