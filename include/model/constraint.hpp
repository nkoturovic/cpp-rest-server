#ifndef CNSTR_HPP
#define CNSTR_HPP

#include <type_traits>
#include <string_view>
#include <any>
#include <concepts/concepts.hpp>

#include <boost/hana.hpp>
namespace hana = boost::hana;

namespace cnstr {

/* Compile type concept (trait) for what is Constraint */
template<typename C>
concept Cnstr = requires(typename C::inner_type t) {
    { C::is_satisfied(t) } -> concepts::same_as<bool>;
    { C::name() } -> concepts::same_as<const char *>;
    { C::description_en() } -> concepts::same_as<std::string>;
    { C::description_rs() } -> concepts::same_as<std::string>;
};

/* --------- Constraints --------- */
struct Required {
    using inner_type = std::any;
    Required() = delete;

    static bool is_satisfied(const std::any &a) { return a.has_value(); } 

    constexpr static const char * name() {
        return "Required";
    }

    static std::string description_en() {
        return "Field is Required";
    }

    static std::string description_rs() {
        return "Polje je obavezno";
    }
};

struct Unique {
    using inner_type = std::any;
    static bool is_satisfied(const std::any &) { 
        return true; 
    }

   constexpr static const char * name() {
        return "Unique";
    }
   static std::string description_en() {
        return "Not available";
   }

   static std::string description_rs() {
        return "Zauzeto";
    }
};
/* ------------ String ----------- */
struct NotEmpty {
    using inner_type = std::string_view;
    NotEmpty() = delete;

    constexpr static bool is_satisfied(std::string_view s) { return !s.empty(); } 
    constexpr static const char * name() { return "NotEmpty"; }

    static std::string description_en() {
        return "Field must not be empty";
    }
    static std::string description_rs() {
        return "Polje ne sme biti prazno";
    }

};

template<int from_ = 0, int to_ = from_>
struct Length { 
    using inner_type = std::string_view;
    Length() = delete;

    constexpr static int from = from_;
    constexpr static int to = to_;

    constexpr static bool is_satisfied(std::string_view s) {
        if ((s.length() < from) || (s.length() > to))
            return false;
        else
            return true;
    }
    constexpr static const char * name() { return "Length"; }

    static std::string description_en() {
        return std::string{"Length should be between "}
             + std::to_string(from)
             + std::string{" and "}
             + std::to_string(to)
             + std::string{" charecters"};
    }
    static std::string description_rs() {
        return std::string{"Duzina mora da bude izmedju "}
             + std::to_string(from)
             + std::string{" i "}
             + std::to_string(to)
             + std::string{" karaktera"};
    }
};

/* ------------ Int ----------- */
template<int from_ = 0, int to_ = from_>
struct Between { 
    using inner_type = int;
    Between() = delete;

    constexpr static int from = from_;
    constexpr static int to = to_;

    constexpr static bool is_satisfied(int x) {
        if ((x < from) || (x > to))
            return false;
        else 
            return true;
    }

    constexpr static const char * name() { return "Between"; }

    static std::string description_en() {
        return std::string{"Value should in range "}
             + std::to_string(from)
             + std::string{" to "}
             + std::to_string(to);
    }

    static std::string description_rs() {
        return std::string{"Vrednost treba da bude u opsegu od "}
             + std::to_string(from)
             + std::string{" do "}
             + std::to_string(to);
    }

};

constexpr auto description = []<Cnstr C>(std::string_view lang = "en") -> std::string {
    if (lang == "rs") {
        return C::description_rs();
    } else /* if lang en */ {
        return C::description_en();
    }
};

constexpr auto name = []<Cnstr C>() -> const char * {
        return C::name();
};

}

#endif // CNSTR_HPP
