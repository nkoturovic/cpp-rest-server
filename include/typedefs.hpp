#ifndef UTIL_HPP
#define UTIL_HPP

#include <variant>
#include <restinio/all.hpp>
#include <nlohmann/json.hpp>

template <typename... Fs>
struct overloaded : Fs... { using Fs::operator()...; };

template <typename... Fs> overloaded(Fs...) -> overloaded<Fs...>;

namespace rs {
using json_t = nlohmann::json;
using router_t = restinio::router::express_router_t<>;
using handler_t = restinio::router::express_request_handler_t;
using api_handler_error_t = std::string;
using api_handler_response_t = std::variant<api_handler_error_t,json_t>;
using api_handler_t = api_handler_response_t(*)(std::optional<json_t>);
}

#endif
