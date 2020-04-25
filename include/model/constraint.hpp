#ifndef RS_CONSTRAINT_HPP
#define RS_CONSTRAINT_HPP

#include <string_view>
#include "3rd_party/static_string.h"
#include <any>

using namespace snw1;

namespace rs::cnstr {

template <typename T>
using ftype = bool(*)(T);

template <auto cstr, typename T, ftype<T> cnstr_func>
struct Constraint {
    //constexpr static auto id = id_;
    constexpr static auto name = cstr;
    constexpr static bool satisfied(const T& t) { 
            return cnstr_func(t); 
    }
};

struct NotEmpty : Constraint<"NotEmpty"_ss, std::string_view, [](std::string_view s) { return s.empty(); }> {};
struct Required : Constraint<"Required"_ss, std::any, [](std::any) { return true; }> {};
struct Unique : Constraint<"Unique"_ss, std::any, [](std::any) { return true; }> {};

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

template <typename T, auto lang = "en"_ss>
constexpr static auto description() {
    if constexpr (lang == "rs"_ss) {
        if constexpr (T::name == Length<>::name) {
            return "Dužina mora biti između " + UTOSS(T::from) + " i " + UTOSS(T::to) + " karaktera";
        } else if constexpr (T::name == NotEmpty::name) {
            return "Polje ne sme biti prazno"_ss;
        } else if constexpr (T::name == Required::name) {
            return "Polje je obavezno"_ss;
        } else {
            return "Nepoznato"_ss;
        }
    } else /* if constexpr (lang == "en"_ss) */ {
        if constexpr (T::name == Length<>::name) {
            return "Length must be between " + UTOSS(T::from) + " and " + UTOSS(T::to) + " characters";
        } else if constexpr (T::name == NotEmpty::name) {
            return "Field can not be empty"_ss;
        } else if constexpr (T::name == Required::name) {
            return "Polje je obavezno"_ss;
        } else {
            return "Unknown"_ss;
        }
    }
}

}


#endif // RS_CONSTRAINT_HPP
