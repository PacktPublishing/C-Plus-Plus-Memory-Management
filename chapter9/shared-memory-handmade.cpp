// also available live: https://wandbox.org/permlink/sIoIJFxVcmv40qJH

//
// OS API header file
//

#include <cstddef> // std::size_t
#include <new> // std::bad_alloc
#include <utility> // std::pair

//
// in most operating systems there is a way
// to create a shared memory zone, identify
// it with some (usually integral) key, then
// address that memory through specialized
// operating system-specific services. We are
// making a "poor person's version" of this
// idea for portability purposes
//

class invalid_shared_mem_key {};

enum shared_mem_id : std::size_t;

//
// creates a shared memory segment of at least
// size contiguous bytes. Returns an identifier
// for that segment. Throws bad_alloc if the
// segment cannot be created
//
shared_mem_id create_shared_mem(std::size_t size);

//
// returns a pair made from the address where a
// shared memory segment begins and the size in
// bytes of that segment, given the identifier
// of the requested segment.
// 
// throws invalid_shared_mem_key if the id does
// not identify an existing shared memory segment
//
std::pair<void*, std::size_t> get_shared_mem(shared_mem_id);

//
// destroys the shared memory segment associated
// with an identifier
// 
// throws invalid_shared_mem_key if the id does
// not identify an existing shared memory segment
// 
// postcondition: associated is memory freed, objects
// therein have their lifetime concluded but are not
// finalized. Use with care!
// 
void destroy_shared_mem(shared_mem_id);

////////////////////////////////////////

//
// OS API .cpp file
//

#include <vector>
#include <memory>
#include <utility>

struct shared_mem_block {
   std::unique_ptr<char[]> mem;
   std::size_t size;
};
std::vector<shared_mem_block> shared_mems;

std::pair<void*, std::size_t> get_shared_mem(shared_mem_id id) {
   if (id < std::size(shared_mems))
      return { shared_mems[id].mem.get(), shared_mems[id].size };
   return { nullptr, 0 };
}
shared_mem_id create_shared_mem(std::size_t size) {
   auto p = std::make_unique<char[]>(size);
   shared_mems.emplace_back(std::move(p), size);
   // note the parentheses
   return shared_mem_id(std::size(shared_mems) - 1);
}

bool is_valid_shared_mem_key(shared_mem_id id) {
   return id < std::size(shared_mems) &&
          shared_mems[id].mem;
}

// postcondition: associated is memory freed, objects
// therein have their lifetime concluded but are not
// finalized. Use with care!
void destroy_shared_mem(shared_mem_id id) {
   if (!is_valid_shared_mem_key(id))
      throw invalid_shared_mem_key{};
   shared_mems[id].mem.reset();
}

////////////////////////////////////////

//
// user code (handmade version)
//

#include <thread>
#include <atomic>
#include <iostream>

struct data {
   bool ready;
   int value;
};

int main() {
   // we need a N-bytes shared memory block
   constexpr std::size_t N = 1'000'000;
   auto key = create_shared_mem(N);
   // map a data object in the shared memory block
   auto [p, sz] = get_shared_mem(key);
   if (!p) return -1;
   // start the lifetime of a non-ready data object
   auto p_data = new (p) data{ false };
   std::atomic<bool> go{ false };
   std::atomic<bool> done{ false };
   std::jthread writer{ [key, &go] {
      go.wait(false);
      auto [p, sz] = get_shared_mem(key);
      if (p) {
         auto p_data = static_cast<data*>(p);
         p_data->value = 3;
         std::atomic_thread_fence(std::memory_order_release);
         p_data->ready = true;
      }
   } };
   std::jthread reader{ [key, &done] {
      auto [p, sz] = get_shared_mem(key);
      if (p) {
         auto p_data = static_cast<data*>(p);
         while (!p_data->ready)
            ; // busy waiting, not cool
         std::cout << "read value " << p_data->value << '\n';
      }
      done = true;
      done.notify_all();
   } };
   if (char c; !std::cin.get(c)) exit(-1);
   go = true;
   go.notify_all();
   // writer and reader run to completion, then complete
   done.wait(false);
   p_data->~data();
   destroy_shared_mem(key);
}
// Cool if you have one data, but what if you have many?
// Who manages it?
