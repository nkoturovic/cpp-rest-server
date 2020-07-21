#include <restinio/all.hpp>
#include <soci/sqlite3/soci-sqlite3.h>

#include "routes.hpp"
#include "typedefs.hpp"
#include "config.hpp"
#include "3rd_party/color.hpp"

using namespace restinio;

int main() 
{
    rs::ServerConfig server_cfg("config/server_config.json");
    soci::session db(soci::sqlite3, "dbname=db.sqlite");
    auto router = std::make_unique<rs::router_t>();

    rs::register_routes(*router, db);

    router->non_matched_request_handler(
        [](auto req) {
            return req->create_response(restinio::status_not_found()).connection_close()
                       .append_header( restinio::http_field::content_type, "application/json" )
                       .set_body(rs::NotFoundError().json().dump())
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
