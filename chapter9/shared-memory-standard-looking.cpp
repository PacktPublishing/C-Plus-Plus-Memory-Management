// also available live: https://wandbox.org/permlink/wdJzJTTR4YcRF6PA

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
// user code (using specialized versions of the
// allocation functions)
//

#include <thread>
#include <atomic>
#include <iostream>

struct data {
   bool ready;
   int value;
};

/////////////////////

#include <algorithm>
#include <iterator>

// what follows is highly inefficient in terms of
// size and speed, but that's besides the point for
// this example
class shared_mem_mgr {
   shared_mem_id key;
   std::vector<bool> taken;
   void *mem;
   auto find_first_free(std::size_t from = 0) {
      using namespace std;
      auto p = find(begin(taken) + from, end(taken), false);
      return distance(begin(taken), p);
   }
   bool at_least_free_from(std::size_t from, int n) {
      using namespace std;
      return from + n < size(taken) &&
             count(begin(taken) + from,
                   begin(taken) + from + n,
                   false) == n;
   }
   void take(std::size_t from, std::size_t to) {
      using namespace std;
      fill(begin(taken) + from, begin(taken) + to, true);
   }
   void free(std::size_t from, std::size_t to) {
      using namespace std;
      fill(begin(taken) + from, begin(taken) + to, false);
   }
public:
   // create shared memory block
   shared_mem_mgr(std::size_t size)
      : key{ create_shared_mem(size) }, taken(size) {
      auto [p, sz] = get_shared_mem(key);
      if (!p) throw invalid_shared_mem_key{};
      mem = p;
   }
   shared_mem_mgr(const shared_mem_mgr&) = delete;
   shared_mem_mgr& operator=(const shared_mem_mgr&) = delete;
   void* allocate(std::size_t n) {
      using namespace std;
      std::size_t i = find_first_free();
      // insanely inefficient
      while (!at_least_free_from(i, n) && i != size(taken))
         i = find_first_free(i + 1);
      if (i == size(taken)) throw bad_alloc{};
      take(i, i + n);
      return static_cast<char*>(mem) + i;
   }
   void deallocate(void *p, std::size_t n) {
      using namespace std;
      auto i = distance(
         static_cast<char*>(mem), static_cast<char*>(p)
      );
      free(i, i + n);
   }
   ~shared_mem_mgr() {
      destroy_shared_mem(key);
   }
};

#include <new>

void* operator new(std::size_t n, shared_mem_mgr& mgr) {
   return mgr.allocate(n);
}
void* operator new[](std::size_t n, shared_mem_mgr& mgr) {
   return mgr.allocate(n);
}
void operator delete(void *p, std::size_t n, shared_mem_mgr& mgr) {
   mgr.deallocate(p, n);
}
void operator delete[](void *p, std::size_t n, shared_mem_mgr& mgr) {
   mgr.deallocate(p, n);
}


//////////////////////////////////////

int main() {
   // we need a N-bytes shared memory block
   constexpr std::size_t N = 1'000'000;
   //// HERE
   shared_mem_mgr mgr{ N };
   // start the lifetime of a non-ready data object
   auto p_data = new (mgr) data{ false };
   std::atomic<bool> go{ false };
   std::atomic<bool> done{ false };
   std::jthread writer{ [p_data, &go] {
      go.wait(false);
      p_data->value = 3;
      std::atomic_thread_fence(std::memory_order_release);
      p_data->ready = true;
   } };
   std::jthread reader{ [p_data, &done] {
      while (!p_data->ready)
         ; // busy waiting, not cool
      std::cout << "read value " << p_data->value << '\n';
      done = true;
      done.notify_all();
   } };
   if (char c; !std::cin.get(c)) exit(-1);
   go = true;
   go.notify_all();
   // writer and reader run to completion, then complete
   done.wait(false);
   p_data->~data();
   operator delete(p_data, sizeof(data), mgr);
}

