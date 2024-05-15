// also available live: https://wandbox.org/permlink/V9ZmbVdMw3lmuAxT

#include <type_traits>
int main() {
   static_assert(std::is_object_v<int>);
   static_assert(!std::is_object_v<decltype(main)>);
}
