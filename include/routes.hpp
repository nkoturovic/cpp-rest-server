#ifndef RS_ROUTES_HPP
#define RS_ROUTES_HPP

#include "handler.hpp"
#include "models.hpp"
#include "utils.hpp"
#include "actions.hpp"

namespace rs {

inline void register_routes(rs::router_t &router, soci::session &db) {

    router.http_get(rs::epr::path_to_params("/api/users"),
            rs::make_handler([&db](rs::model::Empty&&) -> rs::json_t {
                return rs::actions::get_models_from_db<rs::model::User>(db, "users");
            })
    );

    router.http_get(rs::epr::path_to_params("/api/users/", 
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

    router.http_post(rs::epr::path_to_params("/api/users"), 
        rs::make_handler([&db](rs::model::User &&user) -> rs::json_t {
            auto errs = user.unsatisfied_constraints().transform(cnstr::description);
            rs::throw_if<rs::InvalidParamsError>(errs.size(), std::move(errs)); 
            auto duplicates = rs::actions::check_uniquenes_in_db(db, "users", user);
            rs::json_t err_msg; 
            for (const auto &d : duplicates) err_msg[d] = "Already exist in db";
            rs::throw_if<rs::InvalidParamsError>(duplicates.size(), std::move(err_msg));
            rs::actions::insert_model_into_db<rs::model::User>(db, "users", std::move(user));
            return rs::success_response("Model successfully inserted into db");
        })
    );
}

} // ns rs

#endif // RS_ROUTES

