#ifndef RS_FIELD_HPP
#define RS_FIELD_HPP

#include <optional>
#include <vector>
#include <nlohmann/json.hpp>
#include <boost/hana.hpp>
namespace hana = boost::hana;

#include "constraint.hpp"
#include "utils.hpp" // type_name

namespace rs::model {

struct FieldDescription {
    const char * type;
    std::vector<std::string_view> cnstr_names;
};

void to_json(nlohmann::json& j, const FieldDescription& pd) {
    j["type"] = pd.type;
    j["constraints"] = nlohmann::json(pd.cnstr_names);
};

template <typename T, cnstr::Cnstr ...Cs>
struct Field {
    using value_type = T;
    constexpr static auto cnstr_list = hana::tuple_t<Cs...>;
    std::optional<T> opt_value;

    template <cnstr::Cnstr C>
    [[nodiscard]] static consteval bool have_constraint() {
        return hana::contains(cnstr_list, hana::type_c<C>);
    }

    [[nodiscard]] static FieldDescription get_description() {
        return FieldDescription{rs::type_name<value_type>, hana::unpack(cnstr_list, []<typename ...X>(X ...x) {
            return std::vector<std::string_view>{cnstr::get_name.template operator()<typename X::type>()...};
        })}; 
    }

    struct {
        const std::optional<T>& m_value;
        template <class Func, class ... FArgs>
        auto transform(Func && f, FArgs &&...fargs) const {
            if constexpr (std::is_same_v<decltype(f.template operator()<cnstr::Void>(fargs...)), void>) {
                if (m_value) /* If value is set, check all constraints */ {
                    hana::for_each(cnstr_list, [&](auto arg) {
                        using ArgT = typename decltype(arg)::type;
                        if (!ArgT::is_satisfied(*m_value)) {
                            f.template operator()<ArgT>(fargs...);
                        }
                    });
                } else if (hana::contains(cnstr_list, hana::type_c<cnstr::Required>)) {
                    /* value not set but cnstr::Required is in Cs... */
                   f.template operator()<cnstr::Required>(fargs...);
                }
            } else {
                std::vector<decltype(f.template operator()<cnstr::Void>(fargs...))> vec;
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
        }
    } unsatisfied_constraints{opt_value};
};

} // ns rs::model
#endif // RS_FIELD_HPP
