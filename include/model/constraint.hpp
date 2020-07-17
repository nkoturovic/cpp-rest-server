#ifndef CNSTR_HPP
#define CNSTR_HPP

#include <type_traits>
#include <string_view>
#include <any>
#include <concepts>

#include <fmt/compile.h>

#include <boost/hana.hpp>
namespace hana = boost::hana;

namespace cnstr {

/* Compile type concept (trait) for what is Constraint */
template<typename C>
concept Cnstr = requires(typename C::value_type t) {
    { C::is_satisfied(t) } -> std::same_as<bool>;
    { C::name() } -> std::same_as<const char *>;
    { C::description_en() } -> std::same_as<std::string>;
    { C::description_rs() } -> std::same_as<std::string>;
};

/* --------- Constraints --------- */
struct Void {
    using value_type = std::any;
    Void() = delete;

    static bool is_satisfied(const std::any &) { return true; }

    constexpr static const char * name() {
        return "Void";
    }
    static std::string description_en() {
        return "Void Constraint";
    }
    static std::string description_rs() {
        return "Void Constraint";
    }
};


struct Required {
    using value_type = std::any;
    Required() = delete;

    static bool is_satisfied(const std::any &a) { return a.has_value(); } 

    constexpr static const char * name() {
        return "Required";
    }

    static std::string description_en() {
        return "Field should not be empty or invalid";
    }

    static std::string description_rs() {
        return "Polje ne sme biti prazno ili nevalidno";
    }
};

struct Unique {
    using value_type = std::any;
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
    using value_type = std::string_view;
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
    using value_type = std::string_view;
    Length() = delete;

    constexpr static int from = from_;
    constexpr static int to = to_;

    constexpr static bool is_satisfied(std::string_view s) {
        return (s.length() >= from) && (s.length() <= to);
    }
    constexpr static const char * name() { return "Length"; }

    static std::string description_en() {
        return fmt::format("Length should be between {} and {} characters", from, to);
    }
    static std::string description_rs() {
        return fmt::format("Duzina mora da bude izmedju {} i {} karaktera", from, to);
    }
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

    constexpr static const char * name() { return "Between"; }

    static std::string description_en() {
        return fmt::format("Value should be in range {} to {}", from, to);
    }

    static std::string description_rs() {
        return fmt::format("Vrednost treba da bude u opsegu od {} do {}", from, to);
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

/* Maybe change to struct with operator() */
// struct nameT {
//     template <Cnstr C>
//     auto operator()() const {
//         return C::name();
//     }
// };
// 
// struct descriptionT {
//     template <Cnstr C>
//     auto operator()(std::string_view lang = "en") const {
//         if (lang == "rs") {
//             return C::description_rs();
//         } else /* if lang en */ {
//             return C::description_en();
//         }
//     }
// };
// 
// constexpr auto name = nameT{};
// constexpr auto description = descriptionT{};

}

#endif // CNSTR_HPP
