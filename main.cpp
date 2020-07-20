#include <restinio/all.hpp>
#include <soci/sqlite3/soci-sqlite3.h>

#include "handler.hpp"
#include "models.hpp"
#include "typedefs.hpp"
#include "config.hpp"
#include "actions.hpp"
#include "utils.hpp"
#include "3rd_party/color.hpp"

using namespace restinio;

int main() 
{
    rs::ServerConfig server_cfg("config/server_config.json");
    soci::session db(soci::sqlite3, "dbname=db.sqlite");
    auto router = std::make_unique<rs::router_t>();

    router->http_post(rs::epr::path_to_params("/api/user"), 
        rs::make_handler([&db](rs::model::User &&user) -> rs::json_t {
            auto errs = rs::model::apply_to_unsatisfied_cnstrs_of_model(user, cnstr::description);
            rs::throw_if<rs::InvalidParamsError>(errs.size(), std::move(errs)); 
            auto duplicates = rs::actions::check_uniquenes_in_db(db, "users", user);
            rs::json_t err_msg; 
            for (const auto &d : duplicates) err_msg[d] = "Already exist in db";
            rs::throw_if<rs::InvalidParamsError>(duplicates.size(), std::move(err_msg));
            rs::actions::insert_model_into_db<rs::model::User>(db, "users", std::move(user));
            return rs::success_response("Model successfully inserted into db");
        })
    );

    router->http_get(rs::epr::path_to_params("/api/user"),
            rs::make_handler([&db](rs::model::Empty&&) -> rs::json_t {
                return rs::actions::get_models_from_db<rs::model::User>(db, "users");
            })
    );

    router->http_get(rs::epr::path_to_params("/api/user/", 
            rs::epr::non_negative_decimal_number_p<std::uint64_t>()),
            rs::make_handler([&db](rs::model::Id&& req_params, std::uint64_t id) -> rs::json_t {
             if (req_params.id.has_value())
                 std::cout << req_params.id.value() << std::endl;
             else
                 std::cout << "no value" << std::endl;

             auto vec = rs::actions::get_models_from_db<rs::model::User>(db, "users", fmt::format("id = {}", id));
             if (vec.empty())
                 throw rs::NotFoundError("User with that id is not found");
             else 
                 return vec.back();
       })
    );

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
