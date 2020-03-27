#include <restinio/all.hpp>
#include "routes.hpp"
#include "color.hpp"

using namespace restinio;

int main() 
{
    // Create express router for our service.
    auto router = std::make_unique<router::express_router_t<>>();

    for (const auto &route : rs::get_routes()) {
        switch (route->method()) {
            case rs::Method::GET : 
                router->http_get(route->path(), route->gen_callback_func());
            break;

            case rs::Method::POST : 
                router->http_post(route->path(), route->gen_callback_func());
            break;

            case rs::Method::PUT : 
                router->http_put(route->path(), route->gen_callback_func());
            break;

            case rs::Method::DELETE : 
                router->http_delete(route->path(), route->gen_callback_func());
            break;

            case rs::Method::INVALID:
                std::cerr << "Invalid method\n";
        }
    }

    router->non_matched_request_handler(
            [](auto req){
                return req->create_response(restinio::status_not_found()).connection_close()
                .append_header( restinio::http_field::content_type, "application/json" )
                .set_body(R"({ "message" : "Page not found!" })")
                .done();
            });

    // Launching a server with custom traits.
    struct my_server_traits : public default_single_thread_traits_t {
        using request_handler_t = restinio::router::express_router_t<>;
    };

    fmt::print("{}Server running on port {}{}{}\n", 
                  COLOR_GRN, COLOR_YEL, 8080, COLOR_DEF);

    restinio::run(
            restinio::on_this_thread<my_server_traits>()
                    .address("localhost")
                    .request_handler(std::move(router)));

    return 0;
}
