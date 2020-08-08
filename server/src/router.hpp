#ifndef RS_ROUTER_HPP
#define RS_ROUTER_HPP

#include <restinio/router/easy_parser_router.hpp>
#include <boost/hana.hpp>
namespace hana = boost::hana;

#include "utils.hpp"

namespace rs {

class RouteInfo {
public:
    std::string url;
    restinio::http_method_id_t method_id; 
    std::map<std::string, rs::model::ModelFieldDescription> params_description;
};

void to_json(nlohmann::json &j, const RouteInfo& ri) {
    j["url"] = ri.url;
    j["method"] = ri.method_id.c_str();
    j["params"] = ri.params_description;
};

class Router {
private:
    using router_t = restinio::router::easy_parser_router_t;
public:
    std::vector<RouteInfo> registered_routes_info;

    std::unique_ptr<router_t> router_instance;
    Router(std::unique_ptr<router_t> &&router = std::make_unique<router_t>()) : router_instance(std::move(router)) {}

    template<typename MethodMatcher, typename ...PathFragments, typename Handler>
    void add_handler(MethodMatcher &&m, std::tuple<PathFragments...>&& fs, Handler &&handler) {
        namespace epr = restinio::router::easy_parser_router;

        auto url = hana::fold(fs, []<typename F>(std::string &&s, const F &f) {
                if constexpr (std::is_convertible_v<F, const char *>) {
                    return s.append(fmt::format("{}", f));
                } else {
                    return s.append(fmt::format("{}{}{}", '{', F::path_notation, '}'));
                }
        });

        auto cfs = hana::transform(fs, []<typename F>(const F& f) {
                if constexpr (std::is_convertible_v<F, const char *>) {
                    return std::string_view{f};
                } else {
                    return F::produce();
                }
        });

        registered_routes_info.emplace_back(
            url, m, Handler::request_params_model_t::get_description()   
        );

        router_instance->add_handler(std::forward<MethodMatcher>(m), hana::unpack(cfs, [](auto ...xs) {
                    return epr::path_to_params(xs...);
        }), std::forward<Handler>(handler));
    }

    template<typename ...PathFragments, typename Handler>
    void http_get(std::tuple<PathFragments...>&& fs, Handler &&handler) {
        this->add_handler(restinio::http_method_get(), std::move(fs), std::forward<Handler>(handler));
    }

    template<typename ...PathFragments, typename Handler>
    void http_post(std::tuple<PathFragments...>&& fs, Handler &&handler) {
        this->add_handler(restinio::http_method_post(), std::move(fs), std::forward<Handler>(handler));
    }

    template<typename ...PathFragments, typename Handler>
    void http_head(std::tuple<PathFragments...>&& fs, Handler &&handler) {
        this->add_handler(restinio::http_method_head(), std::move(fs), std::forward<Handler>(handler));
    }

    template<typename ...PathFragments, typename Handler>
    void http_put(std::tuple<PathFragments...>&& fs, Handler &&handler) {
        this->add_handler(restinio::http_method_put(), std::move(fs), std::forward<Handler>(handler));
    }

    template<typename ...PathFragments, typename Handler>
    void http_delete(std::tuple<PathFragments...>&& fs, Handler &&handler) {
        this->add_handler(restinio::http_method_delete(), std::move(fs), std::forward<Handler>(handler));
    }

    template <typename Nmrh>
    void non_matched_request_handler(Nmrh &&nmrh) {
       router_instance->non_matched_request_handler(std::forward<Nmrh>(nmrh)); 
    }
};

namespace url {
template <class T>
concept Producer = requires(T t) {
    { T::produce() };
    { T::path_notation } -> std::convertible_to<std::string_view>;
};

template <typename T>
struct non_negative_decimal_number {
    constexpr static auto produce() {
        namespace epr = restinio::router::easy_parser_router;
        return epr::non_negative_decimal_number_p<T>();
    }
    constexpr static std::string_view path_notation = type_name<T>;
};
}

template<typename... Args >
[[nodiscard]] auto make_url(Args && ...args ) {
    return std::make_tuple(std::forward<Args>(args)...);
}

} // ns rs

#endif // RS_ROUTER_HPP
