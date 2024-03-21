// also available live: https://wandbox.org/permlink/5bVdL90OiNzjesqD

#include <thread>
#include <mutex>
#include <string>
#include <algorithm>
#include <string_view>
class string_mutator {
   std::string text;
   mutable std::mutex m;
public:
   // note: m in uncopiable so string_mutator
   // also is uncopiable
   string_mutator(std::string_view src)
      : text{ src.begin(), src.end() } {
   }
   template <class F> void operator()(F f) {
      std::lock_guard lck{ m };
      std::transform(text.begin(), text.end(),
                     text.begin(), f);
   } // implicit m.unlock
   std::string grab_snapshot() const {
      std::lock_guard lck{ m };
      return text;
   } // implicit m.unlock
};
