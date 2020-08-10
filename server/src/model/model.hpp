#ifndef RS_MODEL_HPP
#define RS_MODEL_HPP

#include <iostream>
#include <utility>
#include <concepts>
#include <type_traits>

#include <boost/hana/ext/std/tuple.hpp>
#include <boost/lexical_cast.hpp>
#include <nlohmann/json.hpp>
#include <soci/soci.h>
#include <fmt/format.h>

#include "model/field.hpp"
#include "3rd_party/refl.hpp"
#include "model/constraint.hpp"
#include "utils.hpp" // type_name and lexical_cast from true,false : bool

namespace rs::model {

template <class Derived>
struct Model;

template<typename C>
concept CModel = std::derived_from<C,Model<C>>;

std::ostream& operator<<(std::ostream &out, CModel auto const& model) {
    refl::util::for_each(refl::reflect(model).members, [&](auto member) {
          if constexpr (refl::trait::is_field<decltype(member)>()) {
              if (member(model).has_value()) {
                  out << member.name.c_str() << "=" << member(model).value() << ";";
              }
          }
      });
    return out;
}

void to_json(nlohmann::json& j, CModel auto const& model)
{
    j = nlohmann::json{};
    refl::util::for_each(refl::reflect(model).members, [&](auto member) {
        if constexpr (refl::trait::is_field<decltype(member)>()) {
            if (member(model).has_value()) {
                j[member.name.c_str()] = member(model).value();
            }
        }
    });
}

void from_json(const nlohmann::json& j, CModel auto& model)
{
    refl::util::for_each(refl::reflect(model).members, [&](auto member) {
        using field_type = typename decltype(member)::value_type::value_type;
        if constexpr (refl::trait::is_field<decltype(member)>()) {
            try {
                auto tmp = j.at(member.name.c_str());
                if (!tmp.empty()) {
                    if (!tmp.is_string() || std::is_convertible_v<field_type, std::string>) {
                        member(model).set_value(tmp);
                    } else /* if tmp is string and field_type not conv. to string */ {
                        field_type ftmp = boost::lexical_cast<field_type>(tmp.template get<std::string>()); 
                        member(model).set_value(std::move(ftmp));
                    }
                }
            } catch(...) {
                    // std::clog << __FILE__ 
                        //           << '(' << __LINE__  << ')'
                        //           << ": Polje " << member.name.str() 
                        //           << " nije postavljeno." << '\n';
            }
        }
    });
}

template <typename ...Ts>
class ModelConstraintsWrapper {
private:
    std::tuple<Ts...> cs;
public:
    explicit ModelConstraintsWrapper(std::tuple<Ts...> &&t) : cs(std::move(t)) {}

    template <typename Func, typename ... Args>
    auto transform(Func && f, Args &&...args) const {

        using vec_type = std::vector<decltype(f.template operator()<cnstr::Void>(args...))>;
        std::unordered_map<const char *, vec_type> result_map; result_map.reserve(std::tuple_size_v<decltype(cs)>);
        hana::for_each(cs, [&](auto c) {
            vec_type result_vec = c.second.transform(std::forward<Func>(f), std::forward<Args>(args)...);
            if (result_vec.size())
                 result_map[c.first] = std::move(result_vec);
        });
        return result_map;
    }
};

template <class Derived>
struct Model {
    static auto consteval num_of_fields() { 
        return refl::member_list<Derived>::size;
    }

    static auto consteval field_names() {
        return refl::util::map_to_array<const char *>(refl::member_list<Derived>{}, [](auto member) {
            return member.name.c_str();
        });
    }

    static auto field_names_str() {
        return refl::util::map_to_array<std::string>(refl::member_list<Derived>{}, [](auto member) {
            return member.name.str();
        });
    }

    [[nodiscard]] static auto get_description() {
        std::unordered_map<const char *, FieldDescription> result; result.reserve(num_of_fields());
        refl::util::for_each(refl::member_list<Derived>{}, [&](auto member) {
            if constexpr (refl::trait::is_field<decltype(member)>())
                result[member.name.c_str()] = decltype(member)::value_type::get_description();
        });
        return result;
    }

    template <typename FieldType>
    [[nodiscard]] constexpr FieldType get_field(std::string_view field_name) const {
        auto const& model = static_cast<Derived const&>(*this); FieldType result;
        refl::util::for_each(refl::reflect(model).members, [&](auto member) {
            if (field_name == member.name.c_str())
                result = member(model);
        });
        return result;
    }

    template <typename T>
    bool set_field(std::string_view field_name, T &&value) {
        auto& model = static_cast<Derived&>(*this);
        bool result = false;
        refl::util::for_each(refl::reflect(model).members, [&](auto member) {
            using field_type = typename decltype(member)::value_type::value_type;
            if constexpr (refl::trait::is_field<decltype(member)>()) {
                if (field_name == member.name.c_str() ) {
                    if constexpr (std::is_convertible_v<T, field_type>) {
                        member(model).set_value(std::forward<T>(value));
                        result = true;
                        return;
                    } else {
                        try {
                            field_type casted = boost::lexical_cast<field_type>(std::move(value));
                            member(model).set_value(std::move(casted));
                            result = true;
                        } catch (...) { 
                        }
                        return;
                    }
                }
            }
        });
        return result;
    }

    [[nodiscard]] constexpr auto fields() const {
        auto const& model = static_cast<Derived const&>(*this);
        return refl::util::map_to_tuple(refl::reflect(model).members, [&model](auto member) {
               return member(model);
        });
    }
    
   [[nodiscard]] constexpr auto field_values() const {
        auto const& model = static_cast<Derived const&>(*this);
        return refl::util::map_to_tuple(refl::reflect(model).members, [&model](auto member) {
                return member(model).opt_value();
        });
    }

    [[nodiscard]] constexpr auto field_opt_values_cstr() const {
         auto const& model = static_cast<Derived const&>(*this);
         return refl::util::map_to_array<std::optional<std::string>>(refl::reflect(model).members, [&model](auto member) {
                if (auto field = member(model); field.has_value())
                    return std::make_optional<std::string>(fmt::format("{}", std::move(field.value())));
                else
                    return std::optional<std::string>{std::nullopt};
         });
    }

    [[nodiscard]] auto field_names_values_str() const {
         auto const& model = static_cast<Derived const&>(*this);
         std::vector<std::string> ns; ns.reserve(num_of_fields());
         std::vector<std::string> vs; vs.reserve(num_of_fields());
         refl::util::for_each(refl::reflect(model).members, [&](auto member) {
                if (auto field = member(model); field.has_value()) {
                    ns.emplace_back(member.name.str());
                    vs.emplace_back(fmt::format("{}", std::move(field.value())));
                }
         });
         return std::pair {ns, vs};
    }

    [[nodiscard]] auto get_unsatisfied_constraints() const {
        auto const& model = static_cast<Derived const&>(*this);
        return ModelConstraintsWrapper(refl::util::map_to_tuple(refl::reflect(model).members, [&](auto member) {
                return std::pair{member.name.c_str(), member(model).unsatisfied_constraints};
        }));
    }

    [[nodiscard]] std::unordered_map<const char *,std::string> get_unique_cnstr_fields() const {
        auto const& model = static_cast<Derived const&>(*this);
        std::unordered_map<const char *, std::string> result_map; result_map.reserve(num_of_fields());
        refl::util::for_each(refl::reflect(model).members, [&](auto member) {
            if constexpr (refl::trait::is_field<decltype(member)>()) {
                if (auto field = member(model); field.has_value() && field.unique()) {
                    result_map[member.name.c_str()] = fmt::format("{}", field.value());
                }
            }
        });
        return result_map;
    }

    [[nodiscard]] nlohmann::json json() const {
        nlohmann::json j;
        to_json(j, static_cast<Derived const&>(*this));
        return j;
    }
};

struct Empty final : Model<Empty> {};

void from_json(const nlohmann::json&, Empty&) {};
void to_json(nlohmann::json&, const Empty&) {};

}

REFL_AUTO(
    type(rs::model::Empty)
)

template<rs::model::CModel M>
struct soci::type_conversion<M>
{
    using base_type = soci::values;
    static void from_base(const soci::values &v, soci::indicator& ind, M &model)
    {
        //rs::throw_if<rs::DBError>(ind == soci::i_null, "Null value not allowed for this type");
        refl::util::for_each(refl::reflect(model).members, [&](auto member) {
            if constexpr (refl::trait::is_field<decltype(member)>()) {
                try {
                    using decayed = typename std::remove_cvref_t<decltype(member(model))>::value_type;
                    member(model).set_value(v.get<decayed>(member.name.str()));
                } catch (const std::exception &e) {
                    // Some error (null not okay for this type ...)
                    // std::cerr << member(u).value() << std::endl; // std::cerr << e.what() << std::endl;
                }
            }
        });
    }
    static void to_base(const M &model, soci::values& v, soci::indicator& ind)
    {
        refl::util::for_each(refl::reflect(model).members, [&](auto member) {
            if constexpr (refl::trait::is_field<decltype(member)>()) {
                v.set(member.name.str(), member(model).value());
            }
        });
        ind = soci::indicator::i_ok;
    }
};

#endif //RS_MODEL_BASE_HPP
