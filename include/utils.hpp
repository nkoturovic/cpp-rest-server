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

template <rs::model::CModel M>
constexpr void throw_unsatisfied_constrainsts(const M &m) {

    //auto errs = model::apply_to_unsatisfied_cnstrs_of_model(m, []<cnstr::Cnstr C>() 
    // -> std::map<std::string, std::string> {
    //    return std::map<std::string, std::string> {
    //        { "name", cnstr::name.template operator()<C>() },
    //        { "desc", cnstr::description.template operator()<C>() }
    //    };
    //});
    
    auto errs = model::apply_to_unsatisfied_cnstrs_of_model(m, cnstr::description);

    if (errs.size())
        throw rs::ApiException(ApiErrorId::InvalidParams, errs);
}

template <model::CModel M>
void throw_check_uniquenes_in_db(soci::session &db, std::string_view table_name, const M& m) {
    auto duplicates = rs::actions::check_uniquenes_in_db(db, table_name, m);
    if (duplicates.size()) {
        json_t err_msg;
        for (auto && d : duplicates) {
            err_msg[d] = "Already exist in db";
        }
        throw ApiException(ApiErrorId::InvalidParams, std::move(err_msg));
    }
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
                throw ApiException(ApiErrorId::JsonParseError, perror.what());
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
