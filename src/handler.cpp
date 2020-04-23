#include "handler.hpp"
#include <3rd_party/magic_enum.hpp>

namespace rs {

restinio::request_handling_status_t Handler::operator()(restinio::request_handle_t req, restinio::router::route_params_t params) {
    return do_handle(std::move(req), std::move(params));
}

ApiHandler::ApiHandler(api_handler_t json_handler_func) : m_api_handler(std::move(json_handler_func)) {
                 if (!ApiHandler::s_data) throw "Shared data not initialized!!";
}

void ApiHandler::initialize_shared_data(Shared_data * sd) { ApiHandler::s_data = sd; }

restinio::request_handling_status_t ApiHandler::do_handle(restinio::request_handle_t req, restinio::router::route_params_t params) {

        //////////// HELPER FUNCTIONS ////////////
    auto err_func = [&req, &params](HError &&resp) {
         json_t json;
         json["error_id"] = magic_enum::enum_name(resp.error_id());
         json["error_info"] = resp.extract_json();
         return req->create_response(restinio::status_bad_gateway())
                    .append_header(restinio::http_field::content_type, "application/problem+json")
                    .set_body(json.dump())
                    .done();
    };

     auto succ_func = [&req, &params](HSuccess &&resp) {
         json_t json = resp.extract_json();
         return req->create_response(restinio::status_ok())
                    .append_header(restinio::http_field::content_type, "application/json")
                    .set_body(json.dump())
                    .done();
    };

    auto parse_json = [](auto src) {
        if (src.empty())
            return json_t { nullptr };
        else
            return nlohmann::json::parse(src);
    };
    //////////// END HELPER FUNCTIONS ////////////
    //
    json_t json_req;
    auto req_method = req->header().method();

    if (req_method == restinio::http_method_get()) {
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
    } else if (req_method == restinio::http_method_post()) {
        try {
            json_req = parse_json(req->body());
        } catch (const nlohmann::json::parse_error &perror) {
            return err_func(HError(HError::id::JsonParseError, {}));
        }
    }

    return std::visit(overloaded { succ_func, err_func }, m_api_handler(*s_data, json_req));
}

}
