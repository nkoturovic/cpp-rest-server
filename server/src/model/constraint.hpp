#ifndef CNSTR_HPP
#define CNSTR_HPP

#include <type_traits>
#include <string_view>
#include <any>
#include <concepts>
#include <regex>
#include <fmt/format.h>

namespace rs::model::cnstr {

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

struct ValidImageExtension {
    using value_type = std::string;
    ValidImageExtension() = delete;

    static bool is_satisfied(const std::string& s) {
        const std::regex pattern
                //(R"(([^\s]*(\.(?i)(jpe?g|png|gif|bmp))$))");
                (R"(\.(jpe?g|png|gif|bmp))");
        return std::regex_match(s, pattern);
    }
    constexpr static const char * name = "ValidImageExtension";
    constexpr static const char * description = "image file extension must start with dot(.)"
                                                " and have any one of the following extensions:"
                                                " jpg, jpeg, png, gif, bmp.";
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

template<unsigned long from_ = 0, unsigned long to_ = from_>
struct Length { 
    using value_type = std::string_view;
    Length() = delete;

    constexpr static unsigned long from = from_;
    constexpr static unsigned long to = to_;

    constexpr static bool is_satisfied(std::string_view s) {
        return (s.length() >= from) && (s.length() <= to) ;
    }
    inline static std::string name = fmt::format("Length({},{})", from, to);
    inline static std::string description = fmt::format("Length should be from {} to {}", from, to);
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
template<unsigned long from_ = 0, unsigned long to_ = from_>
struct Between { 
    using value_type = int;
    Between() = delete;

    constexpr static unsigned long from = from_;
    constexpr static unsigned long to = to_;

    constexpr static bool is_satisfied(int x) {
        return (x >= from) && (x <= to);
    }
    inline static std::string name = fmt::format("Between({},{})", from, to);
    inline static std::string description = fmt::format("Value should be in range from {} to {}", from, to);
};

constexpr auto get_description = []<Cnstr C>() -> std::string_view {
    return C::description;
};

constexpr auto get_name = []<Cnstr C>() -> std::string_view {
        return C::name;
};

} // ns rs::model::cnstr

#endif // CNSTR_HPP
