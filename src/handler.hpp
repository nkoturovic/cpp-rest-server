#ifndef RS_HANDLER_HPP
#define RS_HANDLER_HPP

#include "typedefs.hpp"
#include "errors.hpp"
#include "utils.hpp"
#include "model/model.hpp"

namespace rs {

template <class Func, model::CModel RequestParamsModel>
class Handler {
    Func m_handler;
public:
    Handler(Func &&func) : m_handler(std::move(func)) { }
    ~Handler() = default;

        template <typename... RouteParams>
        restinio::request_handling_status_t operator()(restinio::request_handle_t req, RouteParams...routeparams) const {
        try {
            nlohmann::json resp_json;
            nlohmann::json json_req = rs::extract_request_params_model<RequestParamsModel>(req);
            RequestParamsModel pars(std::move(json_req));
            resp_json = m_handler(std::move(pars), routeparams...);

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
             return req->create_response(restinio::status_internal_server_error())
                    .append_header(restinio::http_field::content_type, "application/problem+json")
                    .set_body(rs::DBError(e.get_error_message()).json().dump())
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
