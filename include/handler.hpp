#ifndef RS_HANDLER_HPP
#define RS_HANDLER_HPP

#include "typedefs.hpp"
#include "model/model.hpp"

namespace rs {

/* Parse aditional args: ?name=example ... */
// TODO: Reconsider this
static auto parse_json_params(const auto &req) {
    auto req_method = req->header().method();

    if (req_method == restinio::http_method_get()) {
        json_t json_query_params;
        for (const auto &[k,v] : restinio::parse_query(req->header().query())) {
            auto key_str = fmt::format("{}", k);
            json_query_params[key_str] = std::string(v);
            //try {
            //    json_query_params[key_str] = std::stod(std::string(v));
            //} catch (...) {
            //    if (std::string_view(v) == "true") {
            //        json_query_params[key_str] = 1;
            //    } else if (std::string_view(v) == "false") {
            //        json_query_params[key_str] = 0;
            //    } else {
            //        json_query_params[key_str] = std::string(v);
            //    }
            //}
        }
        return json_query_params;
    } else /* if (req_method == restinio::http_method_post()) */ {
        try {
            auto src = req->body();
            if (src.empty()) {
                return json_t{ nullptr };
            } else {
                return nlohmann::json::parse(src);
            }
        } catch (const nlohmann::json::parse_error &perror) {
            throw ApiException(ApiErrorId::JsonParseError, perror.what());
        }
    }
}

template <class Params, typename ...RouteParams>
class ApiHandler {
    api_handler_t<Params, RouteParams...> m_api_handler;
public:
    ApiHandler(api_handler_t<Params, RouteParams...> &&func) : m_api_handler(std::move(func)) { }
    ~ApiHandler() = default;

    restinio::request_handling_status_t operator()(restinio::request_handle_t req, RouteParams...routeparams) const {
        try {
            json_t json_req = parse_json_params(req);
            //std::cout << "json_params: " << json_req << std::endl;
            //std::cout << "path_params: " << json_t{args...} << std::endl;
            json_t resp_json;
            if constexpr (std::is_same_v<Params, json_t>) {
                 resp_json = m_api_handler(json_req, routeparams...);
            } else if constexpr (std::is_same_v<Params, rs::unit>) {
                 resp_json = m_api_handler(rs::unit{}, routeparams...);
            } else {
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
