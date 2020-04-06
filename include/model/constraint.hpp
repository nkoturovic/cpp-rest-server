#include <string_view>
#include "3rd_party/static_string.h"

using namespace snw1;

namespace cnstr {

template <typename T>
using ftype = bool(*)(T);

template<auto X>
struct cnstr_unique_value {
   static constexpr int value = 0;
};

template <typename... Types>
struct Type_register{};

template <auto cstr, typename T, ftype<T> cstr_func>
struct Constraint {
    //constexpr static auto id = id_;
    constexpr static auto name = cstr;
    constexpr static bool satisfied(const T& t) { return cstr_func(t); }
};

struct Empty : Constraint<"Empty"_ss, bool, [](bool) { return true; }> {};

// Static polymorphism for check function??
template <unsigned from_ = 0u, unsigned to_ = from_>
struct Length : 
    Constraint<"Length"_ss,
              std::string_view, 
              [](std::string_view s) {
                  if ((s.length() < from_) || (s.length() > to_))
                      return false;
                  else
                      return true;
              }> 
{
    constexpr static unsigned from = from_;
    constexpr static unsigned to = to_;
};

template <typename T>
constexpr static auto get_description() {
    if constexpr (T::name == Length<>::name) {
        return "Length must be between " + UTOSS(T::from) + " and " + UTOSS(T::to) + " characters";
    } else {
        return "Some other class";
    }
}

}
