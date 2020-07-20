#ifndef RS_UTILS_HPP
#define RS_UTILS_HPP

#include "typedefs.hpp"
#include "model/model.hpp"
#include "errors.hpp"
#include "actions.hpp"

namespace rs {
json_t success_response(std::string_view info = "") {
    json_t json;
    json["message"] = "SUCCESS";
    if (!info.empty())
        json["info"] = info;
    return json;
};

template <CException E>
constexpr void throw_if(bool condition, json_t &&info = {}) {
    if (condition)
        throw E(std::move(info));
}

/* Parse aditional args (used in handlers.hpp): 
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
                model::set_field(params, key_str, std::string(v));
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

} // ns rs

#endif 
