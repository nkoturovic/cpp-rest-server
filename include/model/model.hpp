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

struct Model {
    virtual ~Model() = default;
};

struct Empty final : Model {
};
void from_json(const nlohmann::json&, Empty&) {};
void to_json(nlohmann::json&, const Empty&) {};

template<typename C>
concept CModel = std::derived_from<C,Model>;

template<CModel M>
struct specialize_model
{
    using base_type = soci::values;
    static void from_base(soci::values const & v, soci::indicator& /* ind */, M &model)
    {
        refl::util::for_each(refl::reflect(model).members, [&](auto member) {
            try {
                using decayed = typename std::remove_cvref_t<decltype(member(model))>::value_type;
                member(model).set_value(v.get<decayed>(member.name.str()));
            } catch (const std::exception &e) {
                // Some error (null not okay for this type ...)
                // std::cerr << member(u).value() << std::endl;
                // std::cerr << e.what() << std::endl;
            }
        });
    }
    static void to_base(const M &model, soci::values& v, soci::indicator& /* ind */)
    {
        refl::util::for_each(refl::reflect(model).members, [&](auto member) {
            v.set(member.name.str(), member(model).value());
        });
    }
};

template <CModel M>
std::ostream& operator<<(std::ostream &out, const M& model) {
    refl::util::for_each(refl::reflect(model).members, [&](auto member) {
          if (member(model).has_value()) {
              out << member.name.str() << "=" << member(model).value() << ";";
          }
      });
    return out;
}

template <CModel M>
void to_json(nlohmann::json& j, const M& model)
{
    j = nlohmann::json{};
    refl::util::for_each(refl::reflect(model).members, [&](auto member) {
        if (member(model).has_value()) {
            j[member.name.str()] = member(model).value();
        }
    });
}

template <CModel M>
void from_json(const nlohmann::json& j, M& model)
{
    refl::util::for_each(refl::reflect(model).members, [&](auto member) {
        using field_type = typename std::remove_cvref_t<decltype(member(model))>::value_type;
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
    });
}

template <CModel M>
constexpr auto fields(M&& model) {
    return refl::util::map_to_tuple(refl::reflect(model).members, [&model](auto member) {
           return member(model);
    });
}

template <CModel M>
constexpr auto field_values(M&& model) {
    return refl::util::map_to_tuple(refl::reflect(model).members, [&model](auto member) {
            return member(model).opt_value();
    });
}

template <CModel M>
constexpr auto field_values_str(M&& model) {
     return refl::util::map_to_array<std::optional<std::string>>(refl::reflect(model).members, [&model](auto member) {
            if (auto field = member(model); field.has_value())
                return std::make_optional<std::string>(fmt::format("{}", std::move(field.value())));
            else
                return std::optional<std::string>{std::nullopt};
     });
}

template <CModel M>
auto fields_with_value_str(M&& model) {

    auto names = field_names(model);
    auto opt_values = field_values_str(std::move(model));
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

template <CModel M>
constexpr auto field_names(const M& model) 
{
    return refl::util::map_to_array<std::string>(refl::reflect(model).members, [](auto member) {
               return member.name.str();
    });
}

template <CModel M, typename T>
bool set_field(M &model, std::string_view field_name, T &&value) {
    bool result = false;
    refl::util::for_each(refl::reflect(model).members, [&](auto member) {
        if (member.name.str() == field_name) {
            using field_type = typename std::remove_cvref_t<decltype(member(model))>::value_type;
            if constexpr (std::is_convertible_v<T, field_type>) {
                member(model).set_value(std::move(value));
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
    });
    return result;
}

template <CModel M>
constexpr auto get_field(M &&model, std::string_view field_name) {
    refl::util::for_each(refl::reflect(model).members, [&](auto member) {
        if (member.name.str() == field_name) {
            return member(model);
        }
    });
}

template <CModel M>
std::map<std::string,std::string> unique_cnstr_fields(M& model)
{
     std::map<std::string, std::string> result_map;
     refl::util::for_each(refl::reflect(model).members, [&](auto member) {
        if (auto field = member(model); field.has_value() && field.unique()) {
            result_map.insert({member.name.str(), fmt::format("{}", field.value())});
        }
     });
    return result_map;
}

template <CModel M, typename Func, typename ... Args>
auto apply_to_unsatisfied_cnstrs_of_model(const M& model, Func &&f, Args&& ...args)
{
    std::map<std::string, std::vector<decltype(f.template operator()<cnstr::Void>(args...))>> results_map;
    refl::util::for_each(refl::reflect(model).members, [&](auto member) {
        if (auto && vec = member(model).apply_to_unsatisfied_cnstrs(f, args...); vec.size()) {
            results_map[member.name.str()] = std::move(vec);
        }
   });
    return results_map;
}

}

#endif //RS_MODEL_BASE_HPP
