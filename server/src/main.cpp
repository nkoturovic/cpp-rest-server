#include <restinio/all.hpp>
#include <soci/sqlite3/soci-sqlite3.h>

#include <span>
#include "router.hpp"
#include "routes.hpp"
#include "utils.hpp"
#include "3rd_party/color.hpp"

using namespace restinio;

int main(int argc, char * argv[])
{
    namespace epr = restinio::router::easy_parser_router;
    const auto [server_address, server_port] = rs::parse_cmdline_args(std::span(argv, argc));

    soci::session db(soci::sqlite3, "dbname=db.sqlite");

    auto router = rs::Router(std::make_unique<restinio::router::easy_parser_router_t>());
    rs::register_routes(router, db);

    router.epr->non_matched_request_handler(
        [](auto req) {
            return req->create_response(restinio::status_not_found()).connection_close()
                       .append_header( restinio::http_field::content_type, "application/json" )
                       .set_body(rs::NotFoundError().json().dump())
                       .done();
    });

    fmt::print("{}Server running on {}{}:{}{}\n", 
                  COLOR_GRN, COLOR_YEL, server_address, server_port, COLOR_DEF);

    using traits_t =
        restinio::traits_t<
            restinio::asio_timer_manager_t,
            restinio::null_logger_t,
            restinio::router::easy_parser_router_t>;

    router.api_get(std::make_tuple("/api/help_json"), 
        [&router](rs::model::Empty) -> nlohmann::json {
            return router.registered_routes_info;
    });

    rs::register_api_reference_route(router, "/api/help");

    restinio::run(restinio::on_thread_pool<traits_t>(16) // Thread pool size is 16 threads.
                 .address(server_address)
                 .port(server_port)
                 .request_handler(std::move(router.epr)));

    return 0;
}
