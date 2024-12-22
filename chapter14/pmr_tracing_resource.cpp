// also available live: https://wandbox.org/permlink/NqyRK2Owgp3MJOu1

#include <print>
#include <iostream>
#include <vector>
#include <string>
#include <memory_resource>
class tracing_resource : public std::pmr::memory_resource {
   void* do_allocate( std::size_t bytes, std::size_t alignment ) override {
       std::print ("do_allocate of {} bytes\n", bytes);
       return upstream->allocate(bytes, alignment);
   }
   void do_deallocate( void* p, std::size_t bytes, std::size_t alignment ) override {
       std::print ("do_deallocate of {} bytes\n", bytes);
       return upstream->deallocate(p, bytes, alignment);
   }
   bool do_is_equal( const std::pmr::memory_resource& other ) const noexcept override {
       return upstream->is_equal(other);
   }
   std::pmr::memory_resource *upstream;
public:
   tracing_resource(std::pmr::memory_resource *upstream) noexcept : upstream{ upstream } {
   }
};
int main() {
   enum { N = 100 };
   tracing_resource tracer{ std::pmr::new_delete_resource() };
   std::pmr::vector<int> v{ &tracer };
   for (int i = 0; i != N; ++i)
      v.emplace_back(i + 1);
   for (auto s : v)
      std::print("{} ", s);
}