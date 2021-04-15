#ifndef RS_ROUTER_HPP
#define RS_ROUTER_HPP

#include <restinio/router/easy_parser_router.hpp>
#include <nlohmann/json.hpp>
#include <boost/hana.hpp>
#include "handler.hpp"
namespace hana = boost::hana;

#include "utils.hpp"

namespace rs {
struct RouteInfo {
    std::string url;
    restinio::http_method_id_t method_id;
    std::unordered_map<const char *, rs::model::FieldDescription> params_description;
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

    std::unique_ptr<router_t> epr;
    explicit Router(std::unique_ptr<router_t> &&router) : epr(std::move(router)) {}

    template<typename MethodMatcher, typename RouteProducer, typename Handler>
    void add_api_handler(MethodMatcher &&m, RouteProducer&& route, Handler &&handler) {
        namespace epr = restinio::router::easy_parser_router;

        auto url = hana::fold(route, []<typename F>(std::string &&s, const F &f) {
                if constexpr (std::is_convertible_v<F, const char *>) {
                    return s.append(fmt::format("{}", f));
                } else {
                    return s.append(fmt::format("{}{}{}", '{', rs::type_name<typename F::result_type>, '}'));
                }
        });

       auto cfs = hana::transform(route, []<typename F>(const F& f) {
               if constexpr (std::is_convertible_v<F, const char *>) {
                   return std::string_view{f};
               } else {
                   return f;
               }
       });

       auto wrapped_handler = make_api_handler(std::forward<Handler>(handler));
       using wrapped_handler_t = decltype(wrapped_handler);

       registered_routes_info.push_back(
           RouteInfo { std::move(url), m, wrapped_handler_t::request_params_model_t::get_description() }
       );

       this->epr->add_handler(std::forward<MethodMatcher>(m), hana::unpack(cfs, [](auto ...xs) {
                    return epr::path_to_params(xs...);
       }), std::move(wrapped_handler));
    }

    template<typename FoldableRoute, typename Handler>
    void api_get(FoldableRoute&& route, Handler &&handler) {
        this->add_api_handler(restinio::http_method_get(), std::forward<FoldableRoute>(route), std::forward<Handler>(handler));
    }

    template<typename FoldableRoute, typename Handler>
    void api_post(FoldableRoute &&route, Handler &&handler) {
        this->add_api_handler(restinio::http_method_post(), std::forward<FoldableRoute>(route), std::forward<Handler>(handler));
    }

    template<typename FoldableRoute, typename Handler>
    void api_head(FoldableRoute &&route, Handler &&handler) {
        this->add_api_handler(restinio::http_method_head(), std::forward<FoldableRoute>(route), std::forward<Handler>(handler));
    }

    template<typename FoldableRoute, typename Handler>
    void api_put(FoldableRoute&& route, Handler &&handler) {
        this->add_api_handler(restinio::http_method_put(), std::forward<FoldableRoute>(route), std::forward<Handler>(handler));
    }

    template<typename FoldableRoute, typename Handler>
    void api_delete(FoldableRoute&& route, Handler &&handler) {
        this->add_api_handler(restinio::http_method_delete(), std::forward<FoldableRoute>(route), std::forward<Handler>(handler));
    }
};
} // ns rs

#endif // RS_ROUTER_HPP
