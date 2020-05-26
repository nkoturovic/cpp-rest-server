#ifndef RS_MODEL_HPP
#define RS_MODEL_HPP

#include <iostream>
#include <utility>
#include <concepts>
#include <type_traits>

#include <nlohmann/json.hpp>
#include <soci/soci.h>

#include "3rd_party/refl.hpp"
#include "model/constraint.hpp"
#include "typedefs.hpp"

namespace rs::model {

struct Model {};

template<typename C>
concept CModel = std::derived_from<C,Model>;

template<CModel M>
struct specialize_model
{
    using base_type = soci::values;
    static void from_base(soci::values const & v, soci::indicator& /* ind */, M &u)
    {
        refl::util::for_each(refl::reflect(u).members, [&](auto member) {
            try {
                if constexpr (refl::trait::is_field<decltype(member)>()) {
                    using decayed = typename std::remove_cvref_t<decltype(member(u))>::value_type;
                    member(u).set_value(v.get<decayed>((member.name.str())));
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
                member(model).set_value(std::move(tmp));
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
                using model_inner_t = typename std::remove_cvref_t<decltype(member(model))>::value_type;
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
                using model_inner_t = typename std::remove_cvref_t<decltype(member(model))>::value_type;
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
auto apply_to_unsatisfied_cnstrs_of_model(const M& model, Func &&f, Args&& ...args)
{
    std::map<std::string, std::vector<decltype(f.template operator()<cnstr::Void>(args...))>> results_map;
    refl::util::for_each(refl::reflect(model).members, [&](auto member) {
        if constexpr (refl::trait::is_field<decltype(member)>()) {
            if (auto && vec = member(model).apply_to_unsatisfied_cnstrs(f, args...); vec.size()) {
                results_map[member.name.str()] = std::move(vec);
            }
        }
   });
    return results_map;
}

}

#endif //RS_MODEL_BASE_HPP
