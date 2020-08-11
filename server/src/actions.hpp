#ifndef RS_ACTIONS_HPP
#define RS_ACTIONS_HPP

#include <fmt/format.h>
#include <fmt/ranges.h>
#include "errors.hpp"
#include "model/model.hpp"

namespace rs::actions {

template <rs::model::CModel M>
std::vector<const char *> check_uniquenes_in_db(soci::session &db, std::string_view table_name, M const& m) {
    std::vector<const char *> duplicates;
    for (auto && [n, v] : m.template field_names_str_values_having_cnstr<model::cnstr::Unique>()) {
        int count = 0;
        db << fmt::format("SELECT COUNT(*) FROM {} WHERE {}='{}'", table_name, n, std::move(v)), soci::into(count);
        if (count) duplicates.push_back(n);
    }
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

void insert_model_into_db(soci::session &db, std::string_view table_name, rs::model::CModel auto &&m) {
    db << fmt::format("INSERT INTO {} ({}) VALUES({})", table_name,
              fmt::join(m.field_names(), ","),
              fmt::join(m.transform_field_values_or(
                  [](const auto &val) {
                      return fmt::format("'{}'", val);
                   }, std::string{"NULL"}), ","));
}

} // ns rs::actions

#endif // RS_ACTIONS_HPP
