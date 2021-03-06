#include <restinio/all.hpp>
#include <soci/sqlite3/soci-sqlite3.h>
#include <soci/connection-pool.h>

#include <span>
#include "router.hpp"
#include "routes.hpp"
#include "utils.hpp"
#include "color.hpp"

using namespace restinio;

int main(int argc, char * argv[])
{
    namespace epr = restinio::router::easy_parser_router;
    auto args = rs::parse_cmdline_args(std::span(argv, static_cast<long unsigned>(argc)));
    auto server_address = args.address.value_or("localhost");
    auto server_port = static_cast<short unsigned int>(args.port.value_or(3000u));

    constexpr std::size_t pool_size = 16;

    soci::connection_pool db_pool(pool_size);
    for (size_t i = 0; i != pool_size; ++i) {
        soci::session& sql = db_pool.at(i);
        sql.open(soci::sqlite3, "dbname=db.sqlite");
    }

    auto router = rs::Router(std::make_unique<restinio::router::easy_parser_router_t>());
    rs::register_routes(router, db_pool);

    router.epr->non_matched_request_handler(
        [](auto req) {
            return req->create_response(restinio::status_not_found()).connection_close()
                       .append_header( restinio::http_field::content_type, "application/json" )
                       .append_header(restinio::http_field::access_control_allow_origin, "*")
                       .append_header(restinio::http_field::access_control_allow_credentials, "true")
                       .set_body(rs::NotFoundError("Route not found").json().dump())
                       .done();
    });

    fmt::print("{}Server running on {}{}:{}{}\n", 
                  COLOR_GRN, COLOR_YEL, server_address, server_port, COLOR_DEF);

    using traits_t =
        restinio::traits_t<
            restinio::asio_timer_manager_t,
            restinio::null_logger_t,
            restinio::router::easy_parser_router_t>;

    router.api_get(std::make_tuple("/help_json"), 
        [&router](rs::model::Empty&&, rs::model::AuthToken&&) -> nlohmann::json {
            return router.registered_routes_info;
    });

    rs::register_api_reference_route(router, "/help");

    restinio::run(restinio::on_thread_pool<traits_t>(pool_size) // Thread pool size is 16 threads.
                 .address(server_address)
                 .port(server_port)
                 .request_handler(std::move(router.epr)));

    return 0;
}
