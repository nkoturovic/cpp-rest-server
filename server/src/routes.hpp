#ifndef RS_ROUTES_HPP
#define RS_ROUTES_HPP

#include <restinio/router/easy_parser_router.hpp>
#include "router.hpp"
#include "handler.hpp"
#include "models.hpp"
#include "utils.hpp"
#include "actions.hpp"

namespace rs {

inline void register_routes(rs::Router &router, soci::session &db) 
{
    namespace epr = restinio::router::easy_parser_router;

   router.api_get(std::make_tuple("/api/users"),
       [&db](rs::model::Empty&&) -> nlohmann::json {
           return rs::actions::get_models_from_db<rs::model::User>(db, "users");
   });

   router.api_get(std::make_tuple("/api/users/", epr::non_negative_decimal_number_p<std::uint32_t>()),
       [&db](rs::model::Id&& req_params, std::uint32_t id) -> nlohmann::json {
           if (req_params.id.opt_value.has_value())
               std::cout << *(req_params.id.opt_value) << std::endl;
           else
               std::cout << "no value" << std::endl;

           auto vec = rs::actions::get_models_from_db<rs::model::User>(db, "users", "*", fmt::format("id = {}", id));
           rs::throw_if<rs::NotFoundError>(vec.empty(), "User with that id is not found");
           return vec.back();
      }
   );

   router.api_post(std::make_tuple("/api/users"), 
      [&db](rs::model::User &&user) -> nlohmann::json {
           auto errs = user.get_unsatisfied_constraints().transform(rs::model::cnstr::get_description);
           rs::throw_if<rs::InvalidParamsError>(!errs.empty(), std::move(errs));
           auto duplicates = rs::actions::check_uniquenes_in_db(db, "users", user);
           nlohmann::json err_msg; 
           for (const auto &d : duplicates) err_msg[d] = "Already exist in db";
           rs::throw_if<rs::InvalidParamsError>(!duplicates.empty(), std::move(err_msg));
           rs::actions::insert_model_into_db<rs::model::User>(db, "users", std::move(user));
           return rs::success_response("Model successfully inserted into db");
       }
   );
}

} // ns rs

#endif // RS_ROUTES

