#ifndef CNSTR_HPP
#define CNSTR_HPP

#include <type_traits>
#include <string_view>
#include <any>
#include <concepts>
#include <regex>

#include <boost/hana.hpp>
namespace hana = boost::hana;

//#include "utils.hpp"

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

namespace rs::model::cnstr {

/* Compile type concept (trait) for what is Constraint */
template<typename C>
concept Cnstr = requires(typename C::value_type t) {
    { C::is_satisfied(t) } -> std::same_as<bool>;
    { C::name } -> std::convertible_to<const char *>;
    { C::description } -> std::convertible_to<const char *>;
};

/* --------- Constraints --------- */
struct Void {
    using value_type = std::any;
    Void() = delete;

    static bool is_satisfied(const std::any &) { return true; }

    constexpr static const char * name = "Void";
    constexpr static const char * description = "Void Constraint";
};


struct Required {
    using value_type = std::any;
    Required() = delete;
    static bool is_satisfied(const std::any &a) { return a.has_value(); } 

    constexpr static const char * name = "Required";
    constexpr static const char * description = "Field should not be empty or invalid";
};

struct Unique {
    using value_type = std::any;
    static bool is_satisfied(const std::any &) { 
        return true; 
    }

   constexpr static const char * name = "Unique";
   constexpr static const char * description = "Unique";
};
/* ------------ String ----------- */
struct NotEmpty {
    using value_type = std::string_view;
    NotEmpty() = delete;

    constexpr static bool is_satisfied(std::string_view s) { return !s.empty(); } 
    constexpr static const char * name = "NotEmpty";
    constexpr static const char * description = "Field must not be empty";
};

struct ValidEmail {
    using value_type = std::string;
    ValidEmail() = delete;

    static bool is_satisfied(const std::string& s) {
        const std::regex pattern
                ("(?:[a-z0-9!#$%&'*+/=?^_`{|}~-]+(?:\\.[a-z0-9!#$%&'*+/=?^_`{|}~-]+)*|\"(?:[\\x01-\\x08\\x0b\\x0c\\"
                 "x0e-\\x1f\\x21\\x23-\\x5b\\x5d-\\x7f]|\\\\[\\x01-\\x09\\x0b\\x0c\\x0e-\\x7f])*\")@(?:(?:[a-z0-9]"
                 "(?:[a-z0-9-]*[a-z0-9])?\\.)+[a-z0-9](?:[a-z0-9-]*[a-z0-9])?|\\[(?:(?:25[0-5]|2[0-4][0-9]|[01]?"
                 "[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?|[a-z0-9-]*[a-z0-9]:(?:[\\x01-\\x08"
                 "\\x0b\\x0c\\x0e-\\x1f\\x21-\\x5a\\x53-\\x7f]|\\\\[\\x01-\\x09\\x0b\\x0c\\x0e-\\x7f])+)\\])");
        return std::regex_match(s, pattern);
    }
    constexpr static const char * name = "ValidEmail";
    constexpr static const char * description = "Email must have @ and dot after";
};

struct ValidPassword {
    using value_type = std::string;
    ValidPassword() = delete;

    static bool is_satisfied(const std::string& s) {
        const std::regex pattern
                (R"(^(?=.*[a-z])(?=.*[A-Z])(?=.*\d)[a-zA-Z\d]{8,}$)");
        return std::regex_match(s, pattern);
    }
    constexpr static const char * name = "ValidPassword";
    constexpr static const char * description = "Minimum eight characters, at least one uppercase letter,"
                                                " one lowercase letter and one number";
};

struct ISOdate {
    using value_type = std::string;
    ISOdate() = delete;

    static bool is_satisfied(const std::string& s) {
        const std::regex pattern
                (R"(([12]\d{3}-(0[1-9]|1[0-2])-(0[1-9]|[12]\d|3[01])))");
        return std::regex_match(s, pattern);
    }
    constexpr static const char * name = "ISOdate";
    constexpr static const char * description = "Date format is yyyy-mm-dd";
};

template<int from_ = 0, int to_ = from_>
struct Length { 
    using value_type = std::string_view;
    Length() = delete;

    constexpr static int from = from_;
    constexpr static int to = to_;

    constexpr static bool is_satisfied(std::string_view s) {
        return (s.length() >= from) && (s.length() <= to) ;
    }
    constexpr static const char * name = (BOOST_HANA_STRING("Length(") + rs::to_hana_string(hana::int_c<from>) + BOOST_HANA_STRING(",")
                                                                          + rs::to_hana_string(hana::int_c<to>) + BOOST_HANA_STRING(")")).c_str();
    constexpr static const char * description = (BOOST_HANA_STRING("Length should be from ") + rs::to_hana_string(hana::int_c<from>) + 
                                                     BOOST_HANA_STRING(" to ") + rs::to_hana_string(hana::int_c<to>)).c_str();
};

struct ValidGender {
    using value_type = std::string_view;
    ValidGender() = delete;

    static bool is_satisfied(std::string_view s) {
        return cnstr::Length<1>::is_satisfied(s) &&
                ( s == "m" || s == "f");
    }
    constexpr static const char * name = "ValidGender";
    constexpr static const char * description = "Gender can be m (male) or f (female)";
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

    constexpr static const char * name = (BOOST_HANA_STRING("Between(") + rs::to_hana_string(hana::int_c<from>) + BOOST_HANA_STRING(",")
                                                                          + rs::to_hana_string(hana::int_c<to>) + BOOST_HANA_STRING(")")).c_str();

    constexpr static const char * description = (BOOST_HANA_STRING("Value should be in range from ") + rs::to_hana_string(hana::int_c<from>) + 
                                                     BOOST_HANA_STRING(" to ") + rs::to_hana_string(hana::int_c<to>)).c_str();
};

constexpr auto get_description = []<Cnstr C>() -> const char * {
    return C::description;
};

constexpr auto get_name = []<Cnstr C>() -> const char * {
        return C::name;
};

} // ns rs::model::cnstr

#endif // CNSTR_HPP
