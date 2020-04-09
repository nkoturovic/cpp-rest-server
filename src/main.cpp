#include <restinio/all.hpp>
#include <soci/sqlite3/soci-sqlite3.h>
#include "typedefs.hpp"
#include "api_routes.hpp"
#include "config.hpp"
#include "color.hpp"

using namespace restinio;

int main() 
{
    rs::ServerConfig server_cfg("config/server_config.json");
    soci::session db(soci::sqlite3, "dbname=ImageShare.sqlite");
    rs::Shared_data sd { db };
    rs::ApiRoute::initialize_shared_data(&sd);

    // Create express router for our service.
    auto router = std::make_unique<rs::router_t>();

    
    for (const auto &route : rs::get_api_routes()) {
        switch (route.method()) {
            case rs::Method::GET : 
                router->http_get(route.path(), std::move(route));
            break;

            case rs::Method::POST : 
                router->http_post(route.path(), std::move(route));
            break;

            case rs::Method::PUT : 
                router->http_put(route.path(), std::move(route));
            break;

            case rs::Method::DELETE : 
                router->http_delete(route.path(), std::move(route));
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

    fmt::print("{}Server running on port {}{}{}\n", 
                  COLOR_GRN, COLOR_YEL, server_cfg.port(), COLOR_DEF);

        using traits_t =
            restinio::traits_t<
                restinio::asio_timer_manager_t,
                restinio::null_logger_t,
                rs::router_t>;

    restinio::run(restinio::on_thread_pool<traits_t>(16) // Thread pool size is 16 threads.
                  .address(server_cfg.ip())
                  .port(server_cfg.port())
                  .request_handler(std::move(router)));

    return 0;
}
