// also available live: https://wandbox.org/permlink/ewIPJZ2qMB0BSFWy

// #include <fstream>
#include <cstdint>
#include <array>
#include <memory>
#include <string_view>

struct Point3D {
   float x{}, y{}, z{};
   Point3D() = default;
   constexpr Point3D(float x, float y, float z)
      : x{ x }, y{ y }, z{ z } {
   }
};


// precondition : dest points to a chunk of at least
// sizeof(float) bytes
unsigned char *to_bytes(unsigned char *dest, float f) {
   unsigned char *b = reinterpret_cast<unsigned char*>(&f);
   auto e = b + sizeof f;
   std::uninitialized_copy(b, e, dest);
   return dest + sizeof f;
}
// precondition : dest points to a chunk of at least
// 3 * sizeof(float) bytes
unsigned char* to_bytes(unsigned char *dest, const Point3D& pt) {
   dest = to_bytes(dest, pt.x);
   dest = to_bytes(dest, pt.y);
   return to_bytes(dest, pt.z);
}

template <int N>
int read_from_stream(std::array<unsigned char, N> &buf,
                     [[maybe_unused]] std::string_view file_name) {
   // let's make it look like we read data
   // from some controlled input source and
   // suppose we are convinced that this data
   // is properly formatted for our needs
   static_assert(sizeof(Point3D) == 3 * sizeof(float));
   static_assert(N >= 4 * sizeof(Point3D)); // for the purpose of our example
   auto p = buf.data();
   p = to_bytes(p, Point3D{ 1.0f, 2.0f, -3.0f });
   p = to_bytes(p, Point3D{ 0, 0, 0 });
   p = to_bytes(p, Point3D{ 1.5f, -2.5f, 1.0f });
   p = to_bytes(p, Point3D{ 1.1f, 2.3f, 3.3f });
   return 4 * sizeof(Point3D);
}

#include <print>
#include <cassert>
using namespace std::literals;
int main() {
   static constexpr int NB_BYTES = 4 * sizeof(Point3D);
   alignas(Point3D) std::array<unsigned char, NB_BYTES> buf{}; // all zeros
   if (int n = read_from_stream<NB_BYTES>(buf, "some_file.dat"sv); n != 0) {
      // print out the bytes: 0-filled left, 2 characters wide, hex format
      for (int i = 0; i != n; ++i)
         std::print("{:0<2x} ", buf[i]);
      std::println();
      // if to treat the bytes as Point3D objects, we need to
      // start the lifetime of the Point3D objects. If we do
      // not, we are in UB territory (it might work or it might
      // not, and even if it works we cannot count on it)
      //
      // in this specific case, with the compiler we are using,
      // reinterpret_cast gives the "correct" results (check it
      // by yourself!)
      const Point3D* pts = std::start_lifetime_as_array(buf.data(), n);
      // const Point3D* pts = reinterpret_cast<Point3D*>(buf.data());
      assert(n % 3 == 0);
      for (std::size_t i = 0; i != n / sizeof(Point3D); ++i)
         std::print("{} {} {}\n", pts[i].x, pts[i].y, pts[i].z);
   }
}
