#include "handler.hpp"
#include <soci/sqlite3/soci-sqlite3.h>
#include <3rd_party/magic_enum.hpp>

namespace rs {

restinio::request_handling_status_t Handler::operator()(restinio::request_handle_t req, restinio::router::route_params_t params) {
    return do_handle(std::move(req), std::move(params));
}

static auto api_parse_query(const auto &req, [[ maybe_unused ]] const auto &params) {
    auto req_method = req->header().method();

    if (req_method == restinio::http_method_get()) {
        json_t json_req;
        for (const auto &[k,v] : restinio::parse_query(req->header().query())) {
                std::stringstream ss;
                ss << k;
                try {
                    json_req[ss.str()] = std::stod(std::string(v));
                } catch (...) {
                    if (std::string(v) == "true") {
                        json_req[ss.str()] = 1;
                    } else if (std::string(v) == "false") {
                        json_req[ss.str()] = 0;
                    } else {
                        json_req[ss.str()] = std::string(v);
                    }
                }
        }
        return json_req;
    } else /* if (req_method == restinio::http_method_post()) */ {
        try {
            auto src = req->body();
            if (src.empty())
                return json_t{ nullptr };
            else
                return nlohmann::json::parse(src);

        } catch (const nlohmann::json::parse_error &perror) {
            throw ApiException(ApiErrorId::JsonParseError, perror.what());
        }
    }
}

restinio::request_handling_status_t ApiHandler::do_handle(restinio::request_handle_t req, restinio::router::route_params_t params) {
    try {
        json_t json_req = api_parse_query(req, params);
        json_t resp_json = m_api_handler(json_req);

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

}
