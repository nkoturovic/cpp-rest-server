#include "handler.hpp"

namespace rs {

restinio::request_handling_status_t Handler::operator()(restinio::request_handle_t req, restinio::router::route_params_t params) {
    return do_handle(std::move(req), std::move(params));
}

ApiHandler::ApiHandler(api_handler_t json_handler_func) : m_api_handler(std::move(json_handler_func)) {
                 if (!ApiHandler::s_data) throw "Shared data not initialized!!";
}

void ApiHandler::initialize_shared_data(Shared_data * sd) { ApiHandler::s_data = sd; }

restinio::request_handling_status_t ApiHandler::do_handle(restinio::request_handle_t req, restinio::router::route_params_t params) {

         api_handler_response_t api_response = m_api_handler(*s_data, { std::nullopt });

        // TODO: Some initial checks
        auto resp_body = std::visit(overloaded{ 
                [](api_handler_error_t err) {
                    return json_t { {"code", "1"}, {"message", err} }.dump();
                },
                [](json_t json) {
                    return json.dump();
                }}, api_response);


         return req->create_response()
        .append_header(restinio::http_field::content_type, "application/json")
        .set_body(resp_body)
        .done();

}

}
