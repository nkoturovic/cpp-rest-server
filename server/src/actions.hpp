#ifndef RS_ACTIONS_HPP
#define RS_ACTIONS_HPP

#include <fmt/format.h>
#include <fmt/ranges.h>
#include "errors.hpp"
#include "model/model.hpp"

namespace rs::actions {

template <rs::model::CModel M>
static auto get_permissions(soci::session &db, std::string_view table_name) 
{
    std::array<std::array<uint8_t, M::num_of_fields()+1>, rs::user::num_of_user_groups> ps { {0} };
    soci::rowset<soci::row> rows = (db.prepare << fmt::format("SELECT * FROM {}", table_name));

    std::for_each(std::begin(rows), std::end(rows),
        [&](const auto &row) {
            auto row_id = row.template get<int>("group_id");
            ps[row_id][0] = row.template get<int>("instance");
            for (auto i = 0u; i < M::num_of_fields(); i++) {
                try {
                    ps[row_id][i+1] = row.template get<int>(M::get_field_name(i));
                } catch (const soci::soci_error &e) {
                    // Ignoring error if field does not exist
                }
            }
    });

    return ps;
}

template <rs::model::CModel M>
std::vector<const char *> check_uniquenes_in_db(soci::session &db, std::string_view table_name, M const& m) {
    std::vector<const char *> duplicates;
    std::apply([&](auto&&... fs) { 
        constexpr auto ns = M::template field_names_having_cnstr<model::cnstr::Unique>();
        auto it = std::begin(ns);
        ((std::invoke(
           [&](auto &&f) { 
              if (f.opt_value.has_value()) {
                  int count = 0;
                  db << fmt::format("SELECT COUNT(*) FROM {} WHERE {}='{}'", table_name, *it, std::move(*f.opt_value)), soci::into(count);
                  if (count) duplicates.push_back(*it);
               };
           }, fs), it++), ...);
    }, m.template fields_having_cnstr<rs::model::cnstr::Unique>());
    return duplicates;
}

template <rs::model::CModel M>
std::vector<M> get_models_from_db(soci::session &db, std::string_view table_name, std::string_view attr = "*", std::string_view filter = "") {
    M m;
    auto filter_stmt = filter.empty() ? "" : fmt::format("WHERE {}", filter);
    soci::statement get_models_stmt = (db.prepare << 
        fmt::format("SELECT {} FROM {} {}", attr, table_name, std::move(filter_stmt)), soci::into(m));
    get_models_stmt.execute();
    std::vector<M> models; models.reserve(get_models_stmt.get_affected_rows());

    while (get_models_stmt.fetch()) {
        models.push_back(std::move(m));
    }

    return models;
}

template <rs::model::CModel M>
void insert_model_into_db(soci::session &db, std::string_view table_name, M &&m) {
    std::array<std::string, M::num_of_fields()> vs;
    auto it = std::begin(vs);
    std::apply([&](auto&&... fs) {
        ((*it = std::invoke([&](auto && f) {
               return f.opt_value.has_value()
                      ? fmt::format("'{}'", std::move(*f.opt_value))
                      : "NULL";
        }, fs), it++), ...);
    }, m.fields());

    db << fmt::format("INSERT INTO {} ({}) VALUES({})", table_name,
              fmt::join(M::field_names(), ","),
              fmt::join(std::move(vs), ","));
}

} // ns rs::actions

#endif // RS_ACTIONS_HPP
