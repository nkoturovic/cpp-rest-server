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
    std::apply([&](const auto&... fs) { 
        constexpr auto ns = M::template field_names_having_cnstr<model::cnstr::Unique>();
        auto it = std::begin(ns);
        ((std::invoke(
           [&](const auto& f) { 
              if (f.opt_value.has_value()) {
                  int count = 0;
                  db << fmt::format("SELECT COUNT(*) FROM {} WHERE {}='{}'", table_name, *it, *f.opt_value), soci::into(count);
                  if (count) duplicates.push_back(*it);
               };
           }, fs), it++), ...);
    }, m.template fields_having_cnstr<rs::model::cnstr::Unique>());
    return duplicates;
}

template <rs::model::CModel M>
std::vector<M> get_models_from_db(const model::AuthToken &auth_tok, PermissionParams pp, soci::session &db, std::string_view table_name, std::string_view attr = "*", std::string_view filter = "") {
    AuthorizedModelAccess model_access(permission::READ, auth_tok, pp, db, table_name, M{});
    std::string filter_stmt = filter.empty() ? "" : fmt::format("WHERE {}", filter);
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
    std::apply([&](const auto&... fs) {
        ((*it = std::invoke([&](const auto& f) {
               return f.opt_value.has_value()
                      ? fmt::format("'{}'", *f.opt_value)
                      : "NULL";
        }, fs), it++), ...);
    }, model_access.move_safely().fields());

    db << fmt::format("INSERT INTO {} ({}) VALUES({})", table_name,
              fmt::join(M::field_names(), ","),
              fmt::join(std::move(vs), ","));
}

template <rs::model::CModel M>
void delete_models_from_db(const model::AuthToken &auth_tok, PermissionParams pp, soci::session &db, std::string_view table_name, std::string_view filter, M &&m) {
    AuthorizedModelAccess model_access(permission::DELETE, auth_tok, pp, db, table_name, std::move(m));
    std::string filter_stmt = filter.empty() ? "" : fmt::format("WHERE {}", filter);
    std::string eq_str; unsigned i =0;
    std::apply([&](const auto&... fs) {
        ((std::invoke([&](const auto& f) {
           if (f.opt_value.has_value()) {
               if (eq_str.empty()) eq_str.append(fmt::format("{}='{}'", M::field_name(i), *f.opt_value));
               else eq_str.append(fmt::format(",{}='{}'", M::field_name(i), *f.opt_value));
           }
        }, fs), i++), ...);
    }, model_access.move_safely().fields());
    rs::throw_if<InvalidParamsError>(eq_str.empty(), "No valid filter parameters");
    db << fmt::format("DELETE FROM {} WHERE {}", table_name, std::move(eq_str), std::move(filter_stmt));
}

template <rs::model::CModel M>
void modify_models_in_db(const model::AuthToken &auth_tok, PermissionParams pp, soci::session &db, std::string_view table_name, std::string_view filter, M &&m) {
    AuthorizedModelAccess model_access(permission::UPDATE, auth_tok, pp, db, table_name, std::move(m));
    std::string filter_stmt = filter.empty() ? "" : fmt::format("WHERE {}", filter);
    std::string set_str; unsigned i =0;
    std::apply([&](const auto&... fs) {
        ((std::invoke([&](const auto& f) {
           if (f.opt_value.has_value()) {
               if (set_str.empty()) set_str.append(fmt::format("{}='{}'", M::field_name(i), *f.opt_value));
               else set_str.append(fmt::format(",{}='{}'", M::field_name(i), *f.opt_value));
           }
        }, fs), i++), ...);
    }, model_access.move_safely().fields());
    rs::throw_if<InvalidParamsError>(set_str.empty(), "No valid parameters to modify");
    db << fmt::format("UPDATE {} SET {} {}", table_name, std::move(set_str), std::move(filter_stmt));
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
