#include <restinio/all.hpp>
#include <soci/sqlite3/soci-sqlite3.h>
#include <soci/connection-pool.h>
#include <cstdlib>

#include <span>
#include "router.hpp"
#include "routes.hpp"
#include "utils.hpp"
#include "3rd_party/color.hpp"

using namespace restinio;

int main(int argc, char * argv[])
{
    namespace epr = restinio::router::easy_parser_router;
    auto argv_span = std::span(argv, argc);
    auto args = rs::parse_cmdline_args(argv_span);

    if (args.help || argv_span.size() <= 1) {
        fmt::print("USAGE {} -d <path_to_db>\n", *argv_span.begin());
        fmt::print("{}", rs::CmdLineArgs::help_string);
        std::exit(0);
    }

    auto server_address = args.address.value_or("localhost");
    auto server_port = args.port.value_or(3000u);
    auto db_config = args.db_config.value_or("db.sqlite");

    constexpr std::size_t pool_size = 16;

    soci::connection_pool db_pool(pool_size);
    for (size_t i = 0; i != pool_size; ++i) {
        soci::session& sql = db_pool.at(i);
        sql.open(soci::sqlite3, fmt::format("dbname={}" ,db_config));
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
