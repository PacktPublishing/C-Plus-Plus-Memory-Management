// also available live: https://wandbox.org/permlink/4KcxDyBbDpsCQCVc

#include <memory>
struct releasable {
   void release() {
      // overly simplified for the
      // sake of this example
      delete this;
   }
protected:
   ~releasable() = default;
};
class important_resource : public releasable {
   // ...
};
void release(releasable *p) {
   if(p) p->release();
}
int main() {
   using namespace std;
   auto p = unique_ptr<important_resource, void(*)(releasable*)>{
      new important_resource, release
   }; // ok, will use release() to delete pointee
   static_assert(sizeof(p) > sizeof(void*));
   auto q = unique_ptr<important_resource,
                       decltype([](auto p) { release(p); })>{
      new important_resource
   };
   static_assert(sizeof(q) == sizeof(void*));
}