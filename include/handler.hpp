#ifndef RS_HANDLER_HPP
#define RS_HANDLER_HPP

#include "typedefs.hpp"
#include "errors.hpp"
#include "model/model.hpp"

namespace rs {

template <typename Params, typename ...RouteParams> 
    requires std::derived_from<Params,model::Model> || std::same_as<Params, rs::Unit>
class ApiHandler {
    api_handler_t<Params, RouteParams...> m_api_handler;

    /* Parse aditional args: for GET: ?name=example ...
     *                       for POST: body */
    static auto extract_params(const auto &req) {
        auto req_method = req->header().method();
        if (req_method == restinio::http_method_get()) {
            Params params;
            for (const auto &[k,v] : restinio::parse_query(req->header().query())) {
                auto key_str = fmt::format("{}", k);
                model::set_field(params, key_str, std::string(v));
            }
            return params;
        } else /* if (req_method == restinio::http_method_post()) */ {
            try {
                auto src = req->body();
                if (src.empty()) {
                    return Params{};
                } else {
                    return Params(nlohmann::json::parse(src));
                }
            } catch (const nlohmann::json::parse_error &perror) {
                throw ApiException(ApiErrorId::JsonParseError, perror.what());
            }
        }
    }

public:
    ApiHandler(api_handler_t<Params, RouteParams...> &&func) : m_api_handler(std::move(func)) { }
    ~ApiHandler() = default;

        restinio::request_handling_status_t operator()(restinio::request_handle_t req, RouteParams...routeparams) const {
        try {
            json_t resp_json;
            if constexpr (std::is_same_v<Params, rs::Unit>) {
                resp_json = m_api_handler(rs::Unit{}, routeparams...);
            } else {
                json_t json_req = this->extract_params(req);
                Params pars(std::move(json_req));
                resp_json = m_api_handler(pars, routeparams...);
            }

        return req->create_response(restinio::status_ok())
                   .append_header(restinio::http_field::content_type, "application/json")
                   .set_body(resp_json.dump())
                   .done();

        } catch(const rs::ApiException &e) {
             return req->create_response(e.status())
                        .append_header(restinio::http_field::content_type, "application/problem+json")
                        .set_body(e.json().dump())
                        .done();
        } catch (const soci::soci_error &e) {
             return req->create_response(restinio::status_internal_server_error())
                    .append_header(restinio::http_field::content_type, "application/problem+json")
                    .set_body(ApiError(ApiErrorId::DBError, e.get_error_message()).json().dump())
                    .done();
        } catch (const std::exception &e) {
             return req->create_response(restinio::status_internal_server_error())
                        .append_header(restinio::http_field::content_type, "application/problem+json")
                        .set_body(ApiError(ApiErrorId::Other, e.what()).json().dump())
                        .done();
        } catch (...) {
            return req->create_response(restinio::status_internal_server_error())
                    .append_header(restinio::http_field::content_type, "application/problem+json")
                    .set_body(R"({ "message" : "Internal error" })")
                    .done();
       }
    }
};

}

#endif // RS_HANDLER_HPP
