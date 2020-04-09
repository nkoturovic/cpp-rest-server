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

namespace model {

template <typename T, typename ...Cs>
class Field {
    template <typename C>
    static void append_if_unsatisfied(std::vector<std::string> &vec, const T &value) {
        if (!C::satisfied(value))
            vec.push_back(cnstr::get_description<C>().str());
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
        }

        return vec;
    }

    bool has_value() const { 
        return m_value.has_value();
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
        std::cout << __FUNCTION__ << " FROM_BASE" << std::endl;
        refl::util::for_each(refl::reflect(u).members, [&](auto member) {
            try {
                if constexpr (refl::trait::is_field<decltype(member)>()) {
                    std::cout << __FUNCTION__ << " IS_VALUE" << std::endl;
                    //if (member(u).has_value()) {
                        std::cout << __FUNCTION__ << " IT_HAS_VALUE" << std::endl;
                        // TODO: Mora uspeti ako smo dosli dovde
                        member(u).value(v.get<typename std::decay<decltype(member(u))>::type::inner_type>((member.name.str())));
                    //}
                }
            } catch (const std::exception &e) {
                std::cerr << member(u).value() << std::endl;
                std::cerr << e.what() << std::endl;
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
        std::cout << "converts" << std::endl;
        if constexpr (refl::trait::is_field<decltype(member)>()) {
            std::cout << "is value" << std::endl;
            if (member(model).has_value()) {
                std::cout << "has value" << std::endl;
                std::cout << member(model).has_value() << std::endl;
                std::cout << member.name.str() << std::endl;
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
                std::cout << "LOG" << std::endl;
                auto tmp = j.at(member.name.str());
                member(model).value(std::move(tmp));
            } catch(...) {
                std::clog << __FILE__ 
                          << '(' << __LINE__  << ')'
                          << ": Polje " << member.name.str() 
                          << " nije postavljeno." << '\n';
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
                using model_inner_t = std::decay<decltype(member(model))>::type::inner_type;
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
            if (member(model).has_value()) {
                if (auto && vec = member(model).check_constraints(); vec.size()) {
                    unsatisfied[member.name.str()] = std::move(vec);
                }
            }
        }
    });
    return unsatisfied;
}

}
