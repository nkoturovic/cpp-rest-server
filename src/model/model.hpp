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

#include "3rd_party/refl.hpp"
#include "model/constraint.hpp"

/* This code is fixing boost::lexical_cast true -> 1 and false -> 0 */
namespace boost {
    template<> 
    bool lexical_cast<bool, std::string>(const std::string& arg) {
        std::istringstream ss(arg);
        bool b;
        ss >> std::boolalpha >> b;
        return b;
    }

    template<>
    std::string lexical_cast<std::string, bool>(const bool& b) {
        std::ostringstream ss;
        ss << std::boolalpha << b;
        return ss.str();
    }
}

namespace rs::model {

template <class Derived>
struct Model;

template<typename C>
concept CModel = std::derived_from<C,Model<C>>;

template<CModel M>
struct specialize_model
{
    using base_type = soci::values;
    static void from_base(soci::values const & v, soci::indicator& /* ind */, M &model)
    {
        refl::util::for_each(refl::reflect(model).members, [&](auto member) {
            if constexpr (refl::trait::is_field<decltype(member)>()) {
                try {
                    using decayed = typename std::remove_cvref_t<decltype(member(model))>::value_type;
                    member(model).set_value(v.get<decayed>(member.name.str()));
                } catch (const std::exception &e) {
                    // Some error (null not okay for this type ...)
                    // std::cerr << member(u).value() << std::endl;
                    // std::cerr << e.what() << std::endl;
                }
            }
        });
    }
    static void to_base(const M &model, soci::values& v, soci::indicator& /* ind */)
    {
        refl::util::for_each(refl::reflect(model).members, [&](auto member) {
            if constexpr (refl::trait::is_field<decltype(member)>()) {
                v.set(member.name.str(), member(model).value());
            }
        });
    }
};

std::ostream& operator<<(std::ostream &out, CModel auto const& model) {
    refl::util::for_each(refl::reflect(model).members, [&](auto member) {
          if constexpr (refl::trait::is_field<decltype(member)>()) {
              if (member(model).has_value()) {
                  out << member.name.str() << "=" << member(model).value() << ";";
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
                j[member.name.str()] = member(model).value();
            }
        }
    });
}

void from_json(const nlohmann::json& j, CModel auto& model)
{
    refl::util::for_each(refl::reflect(model).members, [&](auto member) {
        using field_type = typename std::remove_cvref_t<decltype(member(model))>::value_type;
        if constexpr (refl::trait::is_field<decltype(member)>()) {
            try {
                auto tmp = j.at(member.name.str());
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
        std::map<std::string, vec_type> result_map;
        hana::for_each(cs, [&](auto c) {
            vec_type result_vec = c.second.transform(std::forward<Func>(f), std::forward<Args>(args)...);
            if (result_vec.size())
                 result_map[std::move(c.first)] = std::move(result_vec);
        });
        return result_map;
    }
};

template <class Derived>
struct Model {
    [[nodiscard]] constexpr auto get_field(std::string_view field_name) const {
        auto const& model = static_cast<Derived const&>(*this);
        refl::util::for_each(refl::reflect(model).members, [&](auto member) {
            if constexpr (refl::trait::is_field<decltype(member)>()) {
                if (member.name.str() == field_name) {
                    return member(model);
                }
            }
        });
    }

    template <typename T>
    bool set_field(std::string_view field_name, T &&value) {
        auto& model = static_cast<Derived&>(*this);
        bool result = false;
        refl::util::for_each(refl::reflect(model).members, [&](auto member) {
            using field_type = typename std::remove_cvref_t<decltype(member(model))>::value_type;
            if constexpr (refl::trait::is_field<decltype(member)>()) {
                if (member.name.str() == field_name) {
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

    [[nodiscard]] constexpr auto field_names() const {
        auto const& model = static_cast<Derived const&>(*this);
        return refl::util::map_to_array<std::string>(refl::reflect(model).members, [](auto member) {
                   return member.name.str();
        });
    }

    [[nodiscard]] constexpr auto field_values() const {
        auto const& model = static_cast<Derived const&>(*this);
        return refl::util::map_to_tuple(refl::reflect(model).members, [&model](auto member) {
                return member(model).opt_value();
        });
    }

    [[nodiscard]] auto fields_with_value_str() const {
        auto const& model = static_cast<Derived const&>(*this);
        auto names = model.field_names();
        auto opt_values = model.field_values_str();
        std::vector<std::string> ns; ns.reserve(names.size());
        std::vector<std::string> vs; vs.reserve(names.size());

        for (unsigned i=0; i < names.size(); i++) {
            if (opt_values[i].has_value()) {
                ns.push_back(std::move(names[i]));
                vs.push_back(std::move(opt_values[i].value()));
            }
        }
        return std::pair{ns, vs};
    }

    [[nodiscard]] constexpr auto field_values_str() const {
         auto const& model = static_cast<Derived const&>(*this);
         return refl::util::map_to_array<std::optional<std::string>>(refl::reflect(model).members, [&model](auto member) {
                if (auto field = member(model); field.has_value())
                    return std::make_optional<std::string>(fmt::format("{}", std::move(field.value())));
                else
                    return std::optional<std::string>{std::nullopt};
         });
    }


    [[nodiscard]] auto get_unsatisfied_constraints() const {
        auto const& model = static_cast<Derived const&>(*this);
        return ModelConstraintsWrapper(refl::util::map_to_tuple(refl::reflect(model).members, [&](auto member) {
                return std::pair {member.name.str(), member(model).unsatisfied_constraints};
        }));
    }

    [[nodiscard]] std::map<std::string,std::string> get_unique_cnstr_fields() const {
        auto const& model = static_cast<Derived const&>(*this);
        std::map<std::string, std::string> result_map;
        refl::util::for_each(refl::reflect(model).members, [&](auto member) {
            if constexpr (refl::trait::is_field<decltype(member)>()) {
                if (auto field = member(model); field.has_value() && field.unique()) {
                    result_map.insert({member.name.str(), fmt::format("{}", field.value())});
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

#endif //RS_MODEL_BASE_HPP
