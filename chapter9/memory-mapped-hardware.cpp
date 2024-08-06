// also available live: https://wandbox.org/permlink/SzA6ZtttIFjDvrsM

#include <cstdint>
#include <new>

class super_video_card {
   // ...
public:
   // super duper registers
   volatile std::uint32_t r0{}, r1{}, r2{}, r3{};
   static_assert(sizeof(float) == 4); // sanity check
   volatile float f0{}, f1{}, f2{}, f3{};
   // etc.
   // initialize the video card's state
   super_video_card() = default;
   super_video_card(const super_video_card&) = delete;
   super_video_card& operator=(const super_video_card&) = delete;
   // this would reset the video card's state
   ~super_video_card() = default;
   // various services (omitted for brievity)
};

// somewhere in memory where we have read / write
// access privileges is a memory-mapped hardware
// that corresponds to the actual device

alignas(super_video_card) char
   mem_mapped_device[sizeof(super_video_card)];

void* get_super_card_address() {
   return mem_mapped_device;
}

int main() {
   // map our object to the hardware
   void* p = get_super_card_address();
   auto the_card =
      new(p) super_video_card{ /* args */ };
   // use the_card, use the actual memory-mapped hardware
   // ...
   the_card->~super_video_card();
}
