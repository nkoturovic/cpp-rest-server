#ifndef RS_ACTIONS_HPP
#define RS_ACTIONS_HPP

#include <fmt/format.h>
#include <fmt/ranges.h>
#include "errors.hpp"
#include "model/model.hpp"
#include "user.hpp"
#include <jwt/jwt.hpp>

namespace rs::actions {

template <rs::model::CModel M>
std::vector<const char *> check_uniquenes_in_db(soci::session &db, std::string_view table_name, M const& m) {
    std::vector<const char *> duplicates;
    std::apply([&]<typename ...Fs>(Fs&&... fs) { 
        constexpr auto ns = M::template field_names_having_cnstr<model::cnstr::Unique>();
        auto it = std::begin(ns);
        ((std::invoke(
           [&](const auto &f) { 
              if (f.opt_value.has_value()) {
                  int count = 0;
                  db << fmt::format("SELECT COUNT(*) FROM {} WHERE {}='{}'", table_name, *it, *f.opt_value), soci::into(count);
                  if (count) duplicates.push_back(*it);
               };
           }, std::forward<Fs>(fs)), it++), ...);
    }, m.template fields_having_cnstr<rs::model::cnstr::Unique>());
    return duplicates;
}

template <rs::model::CModel M>
std::vector<M> get_models_from_db(const model::AuthToken &auth_tok, PermissionParams pp, soci::session &db, std::string_view table_name, std::string_view attr = "*", std::string_view filter = "") {
    AuthorizedModelAccess<M> model_access(permission::READ, auth_tok, pp, db, table_name);
    auto filter_stmt = filter.empty() ? "" : fmt::format("WHERE {}", filter);
    soci::statement get_models_stmt = (db.prepare << 
        fmt::format("SELECT {} FROM {} {}", attr, table_name, std::move(filter_stmt)), soci::into(model_access.unsafe_ref()));
    get_models_stmt.execute();
    std::vector<M> models; models.reserve(get_models_stmt.get_affected_rows());

    while (get_models_stmt.fetch()) {
        models.push_back(std::move(model_access.move_safely()));
    }

    return models;
}

template <rs::model::CModel M>
void insert_model_into_db(const model::AuthToken &auth_tok, PermissionParams pp, soci::session &db, std::string_view table_name, M &&m) {
    AuthorizedModelAccess model_access(permission::CREATE, auth_tok, pp, db, table_name, std::move(m));

    std::array<std::string, M::num_of_fields()> vs;
    auto it = std::begin(vs);
    std::apply([&]<typename ...Fs>(Fs&&... fs) {
        ((*it = std::invoke([&](const auto& f) {
               return f.opt_value.has_value()
                      ? fmt::format("'{}'", *f.opt_value)
                      : "NULL";
        }, std::forward<Fs>(fs)), it++), ...);
    }, model_access.move_safely().fields());

    db << fmt::format("INSERT INTO {} ({}) VALUES({})", table_name,
              fmt::join(M::field_names(), ","),
              fmt::join(std::move(vs), ","));
}

model::RefreshAndAuthTokens login(soci::session &db, const model::UserCredentials &credentials) {
    throw_if<InvalidParamsError>(!credentials.username.opt_value.has_value() 
                              || !credentials.password.opt_value.has_value(),
                              "Username or password missing");
    model::User u;
    db << fmt::format("SELECT id,username,password,permission_group FROM users WHERE username = '{}'", *credentials.username.opt_value), soci::into(u);
    throw_if<InvalidParamsError>(*credentials.password.opt_value != *u.password.opt_value, "Invalid username or password");

    jwt::jwt_object auth_token{jwt::params::algorithm("HS256"), jwt::params::secret("changemesecret")};
    auth_token.add_claim("user_id", *u.id.opt_value);
    auth_token.add_claim("group_id", *u.permission_group.opt_value);

    try {
        db << fmt::format("DELETE FROM auth_tokens WHERE user_id = '{}'", *u.id.opt_value);
    } catch (soci::soci_error &) {
        // ignoring if not exists
    }

    db << fmt::format("INSERT INTO auth_tokens (user_id,auth_token) VALUES('{}','{}')", *u.id.opt_value, auth_token.signature());

    jwt::jwt_object refresh_token{jwt::params::algorithm("HS256"), jwt::params::secret("changemesecret") };
    refresh_token.add_claim("user_id", *u.id.opt_value);

    try {
        db << fmt::format("DELETE FROM refresh_tokens WHERE user_id = '{}'", *u.id.opt_value);
    } catch (soci::soci_error &) {
        // ignoring if not exists
    }

    db << fmt::format("INSERT INTO refresh_tokens (user_id,refresh_token) VALUES('{}','{}')", *u.id.opt_value, refresh_token.signature());
    return {.refresh_token = {refresh_token.signature()}, .auth_token = {auth_token.signature()}};
}

} // ns rs::actions

#endif // RS_ACTIONS_HPP
