#ifndef CNSTR_HPP
#define CNSTR_HPP

#include <type_traits>
#include <string_view>
#include <any>
#include <concepts>

//#include "utils.hpp"

#include <boost/hana.hpp>
namespace hana = boost::hana;


namespace rs {
/* Helper for conversion from integral constant to hana string */
constexpr size_t get_magnitude(size_t num) {
  unsigned i = 0;
  while (num > 0) {
    num /= 10;
    ++i;
  }
  return i;
}

template <typename X, size_t ...i>
constexpr auto to_hana_string(X x,
                         std::index_sequence<i...>) {
  constexpr size_t mag = get_magnitude(X::value);
  return hana::string<
    (x / hana::power(hana::size_c<10>,
                     hana::size_c<mag - i - 1>) % hana::size_c<10> 
                       + hana::size_c<48>)...>{};
}

template <typename X>
constexpr auto to_hana_string(X /*x*/) {
  return to_hana_string(hana::size_c<static_cast<size_t>(X::value)>,
                   std::make_index_sequence<get_magnitude(X::value)>());
}
}

namespace rs::cnstr {

/* Compile type concept (trait) for what is Constraint */
template<typename C>
concept Cnstr = requires(typename C::value_type t) {
    { C::is_satisfied(t) } -> std::same_as<bool>;
    { C::name } -> std::convertible_to<std::string_view>;
    { C::description } -> std::convertible_to<std::string_view>;
};

/* --------- Constraints --------- */
struct Void {
    using value_type = std::any;
    Void() = delete;

    static bool is_satisfied(const std::any &) { return true; }

    constexpr static std::string_view name = "Void";
    constexpr static std::string_view description = "Void Constraint";
};


struct Required {
    using value_type = std::any;
    Required() = delete;
    static bool is_satisfied(const std::any &a) { return a.has_value(); } 

    constexpr static std::string_view name = "Required";
    constexpr static std::string_view description = "Field should not be empty or invalid";
};

struct Unique {
    using value_type = std::any;
    static bool is_satisfied(const std::any &) { 
        return true; 
    }

   constexpr static std::string_view name = "Unique";
   constexpr static std::string_view description = "Not Available";
};
/* ------------ String ----------- */
struct NotEmpty {
    using value_type = std::string_view;
    NotEmpty() = delete;

    constexpr static bool is_satisfied(std::string_view s) { return !s.empty(); } 
    constexpr static std::string_view name = "NotEmpty";
    constexpr static std::string_view description = "Field must not be empty";
};

template<int from_ = 0, int to_ = from_>
struct Length { 
    using value_type = std::string_view;
    Length() = delete;

    constexpr static int from = from_;
    constexpr static int to = to_;

    constexpr static bool is_satisfied(std::string_view s) {
        return (s.length() >= from) && (s.length() <= to);
    }
    constexpr static std::string_view name = "Length";
    constexpr static std::string_view description = (BOOST_HANA_STRING("Length should be from ") + rs::to_hana_string(hana::int_c<from>) + 
                                                     BOOST_HANA_STRING(" to ") + rs::to_hana_string(hana::int_c<to>)).c_str();
};

/* ------------ Int ----------- */
template<int from_ = 0, int to_ = from_>
struct Between { 
    using value_type = int;
    Between() = delete;

    constexpr static int from = from_;
    constexpr static int to = to_;

    constexpr static bool is_satisfied(int x) {
        return (x >= from) && (x <= to);
    }

    constexpr static std::string_view name = "Between";
    constexpr static std::string_view description = (BOOST_HANA_STRING("Value should be in range from ") + rs::to_hana_string(hana::int_c<from>) + 
                                                     BOOST_HANA_STRING(" to ") + rs::to_hana_string(hana::int_c<to>)).c_str();
};

constexpr auto get_description = []<Cnstr C>() -> std::string_view {
    return C::description;
};

constexpr auto get_name = []<Cnstr C>() -> std::string_view {
        return C::name;
};

} // ns rs::cnstr

#endif // CNSTR_HPP
