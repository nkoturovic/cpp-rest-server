#ifndef RS_ROUTES_HPP
#define RS_ROUTES_HPP

#include <restinio/router/easy_parser_router.hpp>
#include "router.hpp"
#include "handler.hpp"
#include "models.hpp"
#include "utils.hpp"
#include "actions.hpp"
#include "user.hpp"

namespace rs {

inline void register_routes(rs::Router &router, soci::session &db) 
{
    namespace epr = restinio::router::easy_parser_router;

   router.api_get(std::make_tuple("/users"),
       [&db](rs::model::Empty&&, rs::model::AuthToken &&auth_tok) -> nlohmann::json {
           return rs::actions::get_models_from_db<rs::model::User>(std::move(auth_tok), {.owner_field_name = "id"}, db, "users");
   });

   router.api_get(std::make_tuple("/users/", epr::non_negative_decimal_number_p<std::uint32_t>()),
      [&db](rs::model::Empty&&, rs::model::AuthToken &&auth_tok, std::uint32_t id) -> nlohmann::json {
          auto vec = rs::actions::get_models_from_db<rs::model::User>(std::move(auth_tok),
                         {.owner_field_name = "id"}, db, "users", "*", fmt::format("id = {}", id));
          rs::throw_if<rs::NotFoundError>(vec.empty(), "User with that id is not found");
          return vec.back();
   });

   router.api_post(std::make_tuple("/users"), 
      [&db](rs::model::User &&user, rs::model::AuthToken &&auth_tok) -> nlohmann::json {
          auto errs = user.get_unsatisfied_constraints().transform(rs::model::cnstr::get_description);
          rs::throw_if<rs::InvalidParamsError>(!errs.empty(), std::move(errs));
          auto duplicates = rs::actions::check_uniquenes_in_db(db, "users", user);
          nlohmann::json err_msg; 
          for (const auto &d : duplicates) err_msg[d] = "Already exist in db";
          rs::throw_if<rs::InvalidParamsError>(!duplicates.empty(), std::move(err_msg));
          user.join_date.opt_value = rs::iso_date_now();
          user.permission_group.opt_value = static_cast<int32_t>(UserGroup::user);
          rs::actions::insert_model_into_db(std::move(auth_tok),
              {.owner_field_name = "id"}, db, "users", std::move(user));
          return rs::success_response("Registration sucessfully completed");
   });

   router.api_put(std::make_tuple("/users/", epr::non_negative_decimal_number_p<std::uint32_t>()),
       [&db](rs::model::User&& u, rs::model::AuthToken &&auth_tok, std::uint32_t id) -> nlohmann::json {
           u.get_unsatisfied_constraints().transform(
               []<model::cnstr::Cnstr C>() -> void {
                    if constexpr (std::is_same_v<C, model::cnstr::Required>) {}
                    else { throw InvalidParamsError(C::description); }
           });
           u.id.opt_value = id;
           rs::actions::modify_models_in_db(std::move(auth_tok),
               {.owner_field_name = "id"}, db, "users", fmt::format("id = {}", id), std::move(u));

          return rs::success_response("User informations updated");
   });

   router.api_delete(std::make_tuple("/users/", epr::non_negative_decimal_number_p<std::uint32_t>()),
       [&db](model::Empty&&, rs::model::AuthToken &&auth_tok, std::uint32_t id) -> nlohmann::json {
           rs::model::User u { .id = {id} }; 
           rs::actions::delete_models_from_db(std::move(auth_tok),
               {.owner_field_name = "id"}, db, "users", fmt::format("id = {}", id), std::move(u));

          return rs::success_response(fmt::format("User with id {} deleted", id));
   });

   router.api_post(std::make_tuple("/login"),
       [&db](rs::model::UserCredentials&& cds, rs::model::AuthToken &&auth_tok) -> nlohmann::json {
           throw_if<UnauthorizedError>(auth_tok.auth_token.opt_value.has_value(), "You are already logged in");
           return rs::actions::login(db, cds);
   });

   router.api_get(std::make_tuple("/photos"),
       [&db](rs::model::Empty&&, rs::model::AuthToken &&auth_tok) -> nlohmann::json {
           return rs::actions::get_models_from_db<rs::model::Photo>(std::move(auth_tok), {.owner_field_name = "uploaded_by"}, db, "photos");
   });

   router.epr->http_post(restinio::router::easy_parser_router::path_to_params("/photos"),
      [&db](const restinio::request_handle_t &req) {
         return std::invoke(make_api_handler(
              [&](rs::model::Empty&&, rs::model::AuthToken &&auth_tok) -> nlohmann::json {
                  rs::model::Photo photo = rs::parse_json_field_multiform(req);
                  auto infile = rs::parse_file_field_multiform(req);
                  photo.upload_time.opt_value = rs::iso_date_time_now();
                  photo.extension.opt_value = infile.file_extension;
                  photo.id.opt_value = rs::randint();
                  PermissionParams pp;
                  rs::grant_permission_params_from_auth_token(db, auth_tok, pp); 
                  photo.uploaded_by.opt_value = pp.user_id;
                  auto errs = photo.get_unsatisfied_constraints().transform(rs::model::cnstr::get_description);
                  rs::throw_if<rs::InvalidParamsError>(!errs.empty(), std::move(errs));

                  rs::store_file_to_disk("static/photos/", 
                          std::to_string(*photo.id.opt_value) + *photo.extension.opt_value, infile.file_contents);
                  
                  rs::actions::insert_model_into_db(auth_tok,
                          {.owner_field_name = "uploaded_by"}, db, "photos", std::move(photo));

                  return rs::success_response(std::to_string(*photo.id.opt_value));
              }
          ), req);
   });

   router.api_put(std::make_tuple("/photos/", epr::non_negative_decimal_number_p<std::uint32_t>()),
       [&db](rs::model::Photo&& p, rs::model::AuthToken &&auth_tok, std::uint32_t id) -> nlohmann::json {
           p.get_unsatisfied_constraints().transform(
               []<model::cnstr::Cnstr C>() -> void {
                    if constexpr (std::is_same_v<C, model::cnstr::Required>) {}
                    else { throw InvalidParamsError(C::description); }
           });
           p.id.opt_value = id;

           auto vec = rs::actions::get_models_from_db<rs::model::Photo>(std::move(auth_tok), 
                   {.owner_field_name = "uploaded_by"}, db, "photos", "uploaded_by", fmt::format("id = {}", id));

           throw_if<InvalidParamsError>(vec.empty(), "Photo with that id does not exist");
           p.uploaded_by.opt_value = vec.back().uploaded_by.opt_value;

           rs::actions::modify_models_in_db(std::move(auth_tok),
               {.owner_field_name = "uploaded_by"}, db, "photos", fmt::format("id = {}", id), std::move(p));

          return rs::success_response("Photo informations updated");
   });

   router.api_delete(std::make_tuple("/photos/", epr::non_negative_decimal_number_p<std::uint32_t>()),
       [&db](model::Empty&&, rs::model::AuthToken &&auth_tok, std::uint32_t id) -> nlohmann::json {
           rs::model::Photo p { .id = {id} }; 

           auto vec = rs::actions::get_models_from_db<rs::model::Photo>(std::move(auth_tok), 
                   {.owner_field_name = "uploaded_by"}, db, "photos", "uploaded_by", fmt::format("id = {}", id));

           throw_if<InvalidParamsError>(vec.empty(), "Photo with that id does not exist");
           p.uploaded_by.opt_value = vec.back().uploaded_by.opt_value;

           rs::actions::delete_models_from_db(std::move(auth_tok),
               {.owner_field_name = "uploaded_by"}, db, "photos", fmt::format("id = {}", id), std::move(p));

          return rs::success_response(fmt::format("Photo with id {} deleted", id));
   });
}

} // ns rs

#endif // RS_ROUTES

