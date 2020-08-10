#ifndef RS_FIELD_HPP
#define RS_FIELD_HPP

#include <optional>
#include <vector>
#include <nlohmann/json.hpp>

#include "constraint.hpp"
#include "utils.hpp" // type_name

namespace rs::model {

struct FieldDescription {
    const char * type;
    std::vector<const char *> cnstr_names;
};

void to_json(nlohmann::json& j, const FieldDescription& pd) {
    j["type"] = pd.type;
    j["constraints"] = nlohmann::json(pd.cnstr_names);
};

template <typename T, cnstr::Cnstr ...Cs>
class Field {
private:
    std::optional<T> m_value;
public:
    using value_type = T;
    constexpr static auto cnstr_list = hana::tuple_t<Cs...>;

    [[nodiscard]] static FieldDescription get_description() {
        return FieldDescription{rs::type_name<value_type>, hana::unpack(cnstr_list, []<typename ...X>(X ...x) {
            return std::vector<const char *>{cnstr::get_name.template operator()<typename X::type>()...};
        })}; 
    }

    struct {
        const std::optional<T>& m_value;
        template <class Func, class ... FArgs>
        auto transform(Func && f, FArgs &&...fargs) const {
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
    } unsatisfied_constraints{m_value};

    Field() = default;
    Field(T &&value)
        : m_value(std::move(value)) {}

    constexpr const T& value() const { return m_value.value(); }
    [[nodiscard]] constexpr T&& value() { return std::move(*m_value); }
    constexpr const std::optional<T>& opt_value() const& { return m_value; }
    [[nodiscard]] std::optional<T>&& opt_value() && { return std::move(m_value); }

    void set_value(T&& value) { m_value = std::move(value); }
    void erase_value() { m_value = { std::nullopt }; }
    [[nodiscard]] bool has_value() const { return m_value.has_value(); }

    [[nodiscard]] bool unique() const {
        return hana::contains(cnstr_list, hana::type_c<cnstr::Unique>);
    }
};

} // ns rs::model

#endif // RS_FIELD_HPP
