#include "route.hpp"

using namespace restinio; 

namespace rs {

Route::Route(Method method, std::string path) : m_method(method), m_path(std::move(path)) { }
const std::string& Route::path() const { return m_path; }
Method Route::method() const { return m_method; }
restinio::request_handling_status_t Route::operator()(restinio::request_handle_t req, restinio::router::route_params_t params) {
    return do_handle(std::move(req), std::move(params));
    // auto resp_body = std::visit(overloaded{ 
    //         [](api_handler_error_t err) {
    //             return json_t { {"code", "1"}, {"message", err} }.dump();
    //         },
    //         [](json_t json) {
    //             return json.dump();
    //         }}, api_response);


    // return req->create_response()
    //.append_header(restinio::http_field::content_type, "application/json")
    //.set_body(resp_body)
    //.done();
}


// handler_t ApiRoute::gen_callback_func_impl() const {
//     return [api_handler = std::cref(m_api_handler)](auto req, auto res) -> auto {
// 
//         // set nullopt to actual data
//         api_handler_response_t api_response = api_handler({ std::nullopt });
//         
//         // TODO: Some initial checks
//         auto resp_body = std::visit(overloaded{ 
//                 [](api_handler_error_t err) {
//                     return json_t { {"code", "1"}, {"message", err} }.dump();
//                 },
//                 [](json_t json) {
//                     return json.dump();
//                 }}, api_response);
// 
//         return req->create_response()
//        .append_header(restinio::http_field::content_type, "application/json")
//        .set_body(resp_body)
//        .done();
//     };
// }

ApiRoute::ApiRoute(Method method, std::string path, api_handler_t json_handler_func) :
             Route(method, std::move(path)), m_api_handler(std::move(json_handler_func)) {
                 if (!ApiRoute::s_data) throw "Shared data not initialized!!";
}

void ApiRoute::initialize_shared_data(Shared_data * sd) { ApiRoute::s_data = sd; }

restinio::request_handling_status_t ApiRoute::do_handle(restinio::request_handle_t req, restinio::router::route_params_t params) {

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
