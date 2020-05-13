#ifndef RS_MODEL_BASE_HPP
#define RS_MODEL_BASE_HPP

#include <soci/soci.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <utility>
#include <concepts/concepts.hpp>
#include <type_traits>
#include <fmt/format.h>

#include "3rd_party/refl.hpp"
#include "model/constraint.hpp"
#include "typedefs.hpp"
#include "util.hpp"

namespace rs::model {

template <typename T, cnstr::Cnstr ...Cs>
class Field {
public:
    using inner_type = T;
    constexpr static auto cnstr_list = hana::tuple_t<Cs...>;

    Field() = default;
    Field(T &&value) 
        : m_value(std::forward<T>(value)) {}

    const T& value() const { return m_value.value(); }
    T&& move_value() { return std::move(*m_value); }
    void value(T&& value) { m_value = std::forward<T>(value); }
    void erase_value() { m_value = { std::nullopt }; }

    template <class Func, class ... FArgs>
    auto fmap_unsatisfied(Func && f = Func{}, FArgs&& ... fargs) const { 
        std::vector<decltype(f.template operator()<cnstr::Required>(fargs...))> vec;
        if (m_value) /* If value is set, check all constraints */ {
            hana::for_each(cnstr_list, [&](auto arg) {
                using ArgT = typename decltype(arg)::type;
                if (!ArgT::is_satisfied(*m_value)) {
                    vec.push_back(f.template operator()<ArgT>(fargs...));
                }
            });

        } else if (hana::contains(cnstr_list, hana::type_c<cnstr::Required>)) {
            /* value not set but cnstr::Required is in Cs... */
            vec.push_back(f.template operator()<cnstr::Required>(fargs...));
        }
        return vec;
    }

    bool has_value() const { 
        return m_value.has_value();
    }

    bool unique() const {
        return hana::contains(cnstr_list, hana::type_c<cnstr::Unique>);
    }

private:
    std::optional<T> m_value;
};

struct Model {};

template<typename C>
concept CModel = concepts::derived_from<C,Model>;

template<CModel M>
struct specialize_model
{
    using base_type = soci::values;
    static void from_base(soci::values const & v, soci::indicator& /* ind */, M &u)
    {
        refl::util::for_each(refl::reflect(u).members, [&](auto member) {
            try {
                if constexpr (refl::trait::is_field<decltype(member)>()) {
                    using decayed = typename std::decay<decltype(member(u))>::type::inner_type;
                    member(u).value(v.get<decayed>((member.name.str())));
                }
            } catch (const std::exception &e) {
                // Some error (null not okay for this type ...)
                // std::cerr << member(u).value() << std::endl;
                // std::cerr << e.what() << std::endl;
            }
        });
    }
    static void to_base(const M &u, soci::values& v, soci::indicator& /* ind */)
    {
        refl::util::for_each(refl::reflect(u).members, [&](auto member) {
            if constexpr (refl::trait::is_field<decltype(member)>()) {
                v.set(member.name.str(), member(u).value());
            }
        });
    }
};

template <CModel M>
std::ostream& operator<<(std::ostream &out, M const& m) {
    refl::util::for_each(refl::reflect(m).members, [&](auto member) {
          if constexpr (refl::trait::is_field<decltype(member)>()) {
              if (member(m).has_value()) {
                  out << member.name.str() << "=" << member(m).value() << ";";
              }
          }
      });
    return out;
}

template <CModel M>
void to_json(nlohmann::json& j, const M& model)
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

template <CModel M>
void from_json(const nlohmann::json& j, M& model)
{
    refl::util::for_each(refl::reflect(model).members, [&](auto member) {
        if constexpr (refl::trait::is_field<decltype(member)>()) {
            try {
                auto tmp = j.at(member.name.str());
                member(model).value(std::move(tmp));
            } catch(const nlohmann::json::exception &e) {
                // std::clog << __FILE__ 
                //           << '(' << __LINE__  << ')'
                //           << ": Polje " << member.name.str() 
                //           << " nije postavljeno." << '\n';
            }
        }
    });
}

template <CModel M>
std::map<std::string,std::string> to_map(M& model) 
{
     std::map<std::string, std::string> result_map;
     refl::util::for_each(refl::reflect(model).members, [&](auto member) {
        if constexpr (refl::trait::is_field<decltype(member)>()) {
            if (member(model).has_value()) {
                using model_inner_t = typename std::decay<decltype(member(model))>::type::inner_type;
                if constexpr (std::is_convertible<model_inner_t, std::string>::value) {
                    result_map.insert( {member.name.str(), std::string(member(model).value()) } );
                } else if (std::is_arithmetic<model_inner_t>::value) {
                    result_map.insert( {member.name.str(), std::to_string(member(model).value()) } );
                } else {
                    result_map.insert( {member.name.str(), "ERROR" } );
                }
            }
        }
     });
    return result_map;
}

template <CModel M>
std::map<std::string,std::string> unique_cnstr_fields(M& model)
{
     std::map<std::string, std::string> result_map;
     refl::util::for_each(refl::reflect(model).members, [&](auto member) {
        if constexpr (refl::trait::is_field<decltype(member)>()) {
            if (member(model).has_value() && member(model).unique()) {
                using model_inner_t = typename std::decay<decltype(member(model))>::type::inner_type;
                if constexpr (std::is_convertible<model_inner_t, std::string>::value) {
                    result_map.insert( {member.name.str(), std::string(member(model).value()) } );
                } else if (std::is_arithmetic<model_inner_t>::value) {
                    result_map.insert( {member.name.str(), std::to_string(member(model).value()) } );
                } else {
                    result_map.insert( {member.name.str(), "ERROR" } );
                }
            }
        }
     });
    return result_map;
}

template <CModel M, typename Func, typename ... Args>
auto fmap_unsatisfied_cnstr(const M& model, Func &&f, Args&& ...args)
{
    std::map<std::string, std::vector<decltype(f.template operator()<cnstr::Required>(args...))>> unsatisfied;
    refl::util::for_each(refl::reflect(model).members, [&](auto member) {
        if constexpr (refl::trait::is_field<decltype(member)>()) {
            if (auto && vec = member(model).fmap_unsatisfied(f, args...); vec.size()) {
                unsatisfied[member.name.str()] = std::move(vec);
            }
    }
   });
    return unsatisfied;
}

}

#endif //RS_MODEL_BASE_HPP
