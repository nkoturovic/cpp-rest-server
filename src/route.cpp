#include "route.hpp"

using namespace restinio; 

namespace rs {

Route::Route(Method method, std::string path) : m_method(method), m_path(std::move(path)) { }
const std::string& Route::path() const { return m_path; }
Method Route::method() const { return m_method; }
handler_t Route::gen_callback_func() const { return gen_callback_func_impl(); /* from subclass */ }


handler_t ApiRoute::gen_callback_func_impl() const {
    return [api_handler = std::cref(m_api_handler)](auto req, auto res) -> auto {

        // set nullopt to actual data
        api_handler_response_t api_response = api_handler({ std::nullopt });
        
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
    };
}

ApiRoute::ApiRoute(Method method, std::string path, api_handler_t json_handler_func) :
             Route(method, std::move(path)), m_api_handler(std::move(json_handler_func)) {}

}
