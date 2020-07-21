#ifndef RS_FIELD_HPP
#define RS_FIELD_HPP

#include <optional>
#include <vector>

#include "constraint.hpp"

template <typename T, cnstr::Cnstr ...Cs>
class Field {
public:
    using value_type = T;
    constexpr static auto cnstr_list = hana::tuple_t<Cs...>;

    Field() = default;
    Field(T &&value) 
        : m_value(std::move(value)) {}

    constexpr const T& value() const { return m_value.value(); }
    constexpr T&& value() { return std::move(*m_value); }
    constexpr const std::optional<T>& opt_value() const { return m_value; }
    std::optional<T>&& opt_value() { return std::move(m_value); }

    void set_value(T&& value) { m_value = std::move(value); }
    void erase_value() { m_value = { std::nullopt }; }

    template <class Func, class ... FArgs>
    auto apply_to_unsatisfied_cnstrs(Func && f = Func{}, FArgs&& ... fargs) const { 
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

    bool has_value() const { 
        return m_value.has_value();
    }

    bool unique() const {
        return hana::contains(cnstr_list, hana::type_c<cnstr::Unique>);
    }

private:
    std::optional<T> m_value;
};

#endif // RS_FIELD_HPP
