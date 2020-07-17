#include <restinio/all.hpp>
#include <soci/sqlite3/soci-sqlite3.h>

#include "handler.hpp"
#include "model/models.hpp"
#include "handlers.hpp"
#include "typedefs.hpp"
#include "config.hpp"
#include "color.hpp"
#include "actions.hpp"

using namespace restinio;

int main() 
{
    rs::ServerConfig server_cfg("config/server_config.json");
    soci::session db(soci::sqlite3, "dbname=db.sqlite");
    auto router = std::make_unique<rs::router_t>();

    router->http_post(rs::epr::path_to_params("/api/user"), 
            rs::ApiHandler<rs::model::User>(rs::handlers::insert_model_into_db<rs::model::User>(db, "users")));

    router->http_get(rs::epr::path_to_params("/api/user"),
            rs::ApiHandler<rs::unit>(rs::handlers::get_models_from_db<rs::model::User>(db, "users")));

   router->http_get(rs::epr::path_to_params("/api/user/", 
           rs::epr::non_negative_decimal_number_p<std::uint64_t>()),
           rs::ApiHandler<rs::handler_params::HPars_get_model_by_id, std::uint64_t>(rs::handlers::get_user_by_id(db)));

    router->non_matched_request_handler(
            [](auto req) {
                return req->create_response(restinio::status_not_found()).connection_close()
                .append_header( restinio::http_field::content_type, "application/json" )
                .set_body(rs::ApiError(rs::ApiErrorId::NotFound).json().dump())
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
