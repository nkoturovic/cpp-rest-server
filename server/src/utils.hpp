#ifndef RS_UTILS_HPP
#define RS_UTILS_HPP

#include <span>
#include <restinio/router/easy_parser_router.hpp>

#include "model/model.hpp"
#include "errors.hpp"
#include "actions.hpp"

#include <boost/hana.hpp>
namespace hana = boost::hana;

namespace rs {

nlohmann::json success_response(std::string_view info = "") {
    nlohmann::json json;
    json["message"] = "SUCCESS";
    if (!info.empty())
        json["info"] = info;
    return json;
};

template <CError E>
constexpr void throw_if(bool condition, nlohmann::json &&info = {}) {
    if (condition)
        throw E(std::move(info));
}

/* Parse additional args (used in handlers.hpp):
 * for GET: ?name=example ...
 * for POST: body */
template <model::CModel RequestParamsModel>
RequestParamsModel extract_request_params_model(const auto &req) {
    if constexpr (std::is_same_v<RequestParamsModel, model::Empty>) {
        return model::Empty{};
    } else { 
        auto req_method = req->header().method();
        if (req_method == restinio::http_method_get()) {
            RequestParamsModel params;
            for (const auto &[k,v] : restinio::parse_query(req->header().query())) {
                auto key_str = fmt::format("{}", k);
                params.set_field(key_str, std::string(v));
            }
            return params;
        } else /* if (req_method == restinio::http_method_post()) */ {
            try {
                auto src = req->body();
                if (src.empty()) {
                    return RequestParamsModel{};
                } else {
                    return RequestParamsModel(nlohmann::json::parse(src));
                }
            } catch (const nlohmann::json::parse_error &perror) {
                throw rs::JsonParseError(perror.what());
            }
        }
    }
}

struct CmdLineArgs {
    const char * ip = "localhost";
    unsigned port = 3000;
};

CmdLineArgs parse_cmdline_args(std::span<char *> args) 
{
    CmdLineArgs result{};
    auto it_end = std::cend(args);

    for (auto it = std::cbegin(args); it != it_end; it++) {
        auto it_next = std::next(it);
        std::string_view curr{*it};
        if ((curr == "--ip" || curr == "-i") && it_next != it_end)
            result.ip = *it_next;
        else if ((curr == "--port" || curr == "-p") && it_next != it_end)
            result.port = std::strtoul(*it_next, nullptr, 10);
    }
    return result;
};

template <typename T>
struct function_traits
    : public function_traits<decltype(&T::operator())>
{};
// For generic types, directly use the result of the signature of its 'operator()'

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) const>
// we specialize for pointers to member function
{
    enum { arity = sizeof...(Args) };
    // arity is the number of arguments.

    using result_type = ReturnType;

    template <size_t i>
    struct arg
    {
        using type = typename std::tuple_element<i, std::tuple<Args...>>::type;
        // the i-th argument is equivalent to the i-th tuple element of a tuple
        // composed of those arguments.
    };
};

void register_api_reference_route(const auto &router) {
    router.router_instance->http_get(restinio::router::easy_parser_router::path_to_params("/api/help"), 
            [&](const auto &req) {
                std::string content = {"<!DOCTYPE html><html></body><h1>Api reference</h1>"};
                for (const auto &r : router.registered_routes_info) {
                    content.append(fmt::format("<b>{}</b>:&nbsp;&nbsp;{}<br><code>", r.method_id, r.url));
                    for (const auto &[k,v] : r.params_description) {
                        content.append(fmt::format("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{} : {} &lt;", k, v.type));
                        if (v.cnstr_names.begin() != v.cnstr_names.end()) {
                            content.append(std::accumulate(v.cnstr_names.begin() + 1, 
                                            v.cnstr_names.end(), 
                                            std::string{*v.cnstr_names.begin()}, 
                                            [](std::string acc, std::string_view s) {
                                               return acc + ", " + std::string(s); 
                                            }
                            ));
                        }
                        content.append("&gt;<br>");
                    }
                    content.append("</code><br>");
                }
                content.append("</body></html>");
                return req->create_response(restinio::status_ok())
                   .append_header(restinio::http_field::content_type, "text/html")
                   .set_body(content)
                   .done();
    });
}

} // ns rs

#endif 
