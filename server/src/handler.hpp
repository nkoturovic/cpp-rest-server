#ifndef RS_HANDLER_HPP
#define RS_HANDLER_HPP

#include "errors.hpp"
#include "utils.hpp"
#include "model/model.hpp"

namespace rs {

/* Parse additional args (used in handlers.hpp):
 * for GET: ?name=example ...
 * for POST: body */
template <model::CModel RequestParamsModel>
static inline RequestParamsModel extract_request_params_model(const auto &req) {
    if constexpr (std::is_same_v<RequestParamsModel, model::Empty>) {
        return model::Empty{};
    } else { 
        auto req_method = req->header().method();
        if (req_method == restinio::http_method_get()) {
            RequestParamsModel params;
            for (const auto &[k,v] : restinio::parse_query(req->header().query())) {
                auto key_str = fmt::format("{}", k);
                params.try_set_field_value(key_str, std::string(v));
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

template <class Func, model::CModel RequestParamsModel>
class Handler {
    Func m_handler;
public:
    using request_params_model_t = RequestParamsModel;

    explicit Handler(Func &&func) : m_handler(std::move(func)) { }
    ~Handler() = default;

        template <typename... RouteParams>
        restinio::request_handling_status_t operator()(const restinio::request_handle_t &req, RouteParams&& ...routeparams) const {
        try {
            nlohmann::json resp_json;
            nlohmann::json json_req = rs::extract_request_params_model<RequestParamsModel>(req);
            RequestParamsModel pars(std::move(json_req));
            resp_json = m_handler(std::move(pars), std::forward<RouteParams>(routeparams)...);

        return req->create_response(restinio::status_ok())
                   .append_header(restinio::http_field::content_type, "application/json")
                   .set_body(resp_json.dump())
                   .done();

        } catch(const rs::Error &e) {
             return req->create_response(e.status())
                        .append_header(restinio::http_field::content_type, "application/problem+json")
                        .set_body(e.json().dump())
                        .done();
        } catch (const soci::soci_error &e) {
            // TODO: Put this custom messages - It Yields Unknown DB error for Unique Constraint violation 
            // constexpr auto msg_from_category = [](soci::soci_error::error_category category) {
            //     switch (category) {
            //         case soci::soci_error::connection_error:
            //             return "Connection error";
            //         case soci::soci_error::invalid_statement:
            //             return "Invalid statement";
            //         case soci::soci_error::no_privilege:
            //             return "Invalid privilege";
            //         case soci::soci_error::no_data:
            //             return "No data";
            //         case soci::soci_error::constraint_violation:
            //             return "Constraint violation";
            //         case soci::soci_error::unknown_transaction_state:
            //             return "Unknown transaction state";
            //         case soci::soci_error::system_error:
            //             return "System Error";
            //         default: /* soci::soci_error::unknown: */
            //             return "Unknown DB error";
            //     }
            // };
            // // Maybe log somewhere: e.get_error_message(); or e.what();
            // const char * msg = msg_from_category(e.get_error_category());
             return req->create_response(restinio::status_internal_server_error())
                    .append_header(restinio::http_field::content_type, "application/problem+json")
                    .set_body(rs::DBError(/*TODO:msg*/e.get_error_message()).json().dump())
                    .done();
        } catch (const std::exception &e) {
             return req->create_response(restinio::status_internal_server_error())
                        .append_header(restinio::http_field::content_type, "application/problem+json")
                        .set_body(rs::OtherError(e.what()).json().dump())
                        .done();
        } catch (...) {
            return req->create_response(restinio::status_internal_server_error())
                    .append_header(restinio::http_field::content_type, "application/problem+json")
                    .set_body(rs::OtherError().json().dump())
                    .done();
       }
    }
};

template <class Func>
auto make_handler(Func &&f)
{
    using traits = rs::function_traits<Func>;
    using arg_type = typename traits:: template arg<0>::type;
    return Handler<Func, std::remove_cvref_t<arg_type>>(std::forward<Func>(f));
}

} // ns rs

#endif // RS_HANDLER_HPP
