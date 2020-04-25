#ifndef RS_DATA_MODEL_BASE_HPP
#define RS_DATA_MODEL_BASE_HPP

#include <soci/soci.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <utility>
#include <concepts/concepts.hpp>
#include <type_traits>
#include "3rd_party/refl.hpp"
#include <fmt/format.h>
#include "model/constraint.hpp"
#include <optional>
#include "typedefs.hpp"

#include <type_traits>

template < typename Tp, typename... List >
struct contains : std::true_type {};

template < typename Tp, typename Head, typename... Rest >
struct contains<Tp, Head, Rest...>
: std::conditional< std::is_same<Tp, Head>::value,
    std::true_type,
    contains<Tp, Rest...> >::type {};

template < typename Tp >
struct contains<Tp> : std::false_type {};

namespace rs::model {

template <typename T, typename ...Cs>
class Field {
    template <typename C>
    static void append_if_unsatisfied(std::vector<std::string> &vec, const T &value) {
        if (!C::satisfied(value))
            vec.push_back(cnstr::description<C>().str());
    }
public:
    using inner_type = T;

    Field() = default;
    Field(T value) 
        : m_value(std::forward<T>(value)) {}

    const T& value() const { return m_value.value(); }
    T&& move_value() { return std::move(*m_value); }
    void value(T&& value) { m_value = std::forward<T>(value); }
    void erase_value() { m_value = { std::nullopt }; }

    std::vector<std::string> check_constraints() const { 
        std::vector<std::string> vec;

        if (m_value) {
            (append_if_unsatisfied<Cs>(vec, m_value.value()), ...);
        } else if (contains<cnstr::Required, Cs...>::value) {
            vec.push_back(cnstr::description<cnstr::Required>().str());
        }

        return vec;
    }

    bool has_value() const { 
        return m_value.has_value();
    }

    bool unique() const {
        return contains<cnstr::Unique, Cs...>::value;
    }

private:
    std::optional<T> m_value;
};

template <typename ...Cs>
using Text = Field<std::string, Cs...>;

template <typename ...Cs>
using Integer = Field<int, Cs...>;

struct Model {
};

template<class U> requires concepts::derived_from<U,Model> 
struct specialize_model
{
    using base_type = soci::values;
    static void from_base(soci::values const & v, soci::indicator /* ind */, U & u)
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
    static void to_base(const U & u, soci::values & v, soci::indicator & ind)
    {
        refl::util::for_each(refl::reflect(u).members, [&](auto member) {
            if constexpr (refl::trait::is_field<decltype(member)>()) {
                v.set(member.name.str(), member(u).value());
            }
        });
    }
};

template <typename M>
std::ostream& operator<<(std::ostream &out, const M &m) requires concepts::derived_from<M,Model> {
    refl::util::for_each(refl::reflect(m).members, [&](auto member) {
          if constexpr (refl::trait::is_field<decltype(member)>()) {
              if (member(m).has_value()) {
                  out << member.name.str() << "=" << member(m).value() << ";";
              }
          }
      });
    return out;
}

template <typename M>
void to_json(nlohmann::json& j, const M& model) requires concepts::derived_from<M,Model>
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

template <typename M>
void from_json(const nlohmann::json& j, M& model) requires concepts::derived_from<M,Model> 
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

template <typename M>
std::map<std::string,std::string> to_map(M& model) requires concepts::derived_from<M,Model>  {
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

template <typename M>
std::map<std::string,std::string> unique_cnstr_fields(M& model) requires concepts::derived_from<M,Model>  {
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

template <typename M>
std::map<std::string, std::vector<std::string>> unsatisfied_constraints(const M& model) requires concepts::derived_from<M,Model>
{
    std::map<std::string, std::vector<std::string>> unsatisfied;
    refl::util::for_each(refl::reflect(model).members, [&](auto member) {
        if constexpr (refl::trait::is_field<decltype(member)>()) {
            if (auto && vec = member(model).check_constraints(); vec.size()) {
                unsatisfied[member.name.str()] = std::move(vec);
            }
    }
   });
    return unsatisfied;
}

}

#endif //RS_DATA_MODEL_BASE_HPP
