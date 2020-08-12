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
    std::apply([&](auto&&... fs) { 
        constexpr auto ns = M::template field_names_having_cnstr<model::cnstr::Unique>();
        auto i = 0u;
        ((rs::if_else(fs.opt_value.has_value(),
            [&](auto &&f) { 
                int count = 0;
                db << fmt::format("SELECT COUNT(*) FROM {} WHERE {}='{}'", table_name, ns[i], std::move(*f.opt_value)), soci::into(count);
                if (count) duplicates.push_back(ns[i]);
            },
            [&](auto &&) {},
        fs), i++), ...);
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

void insert_model_into_db(soci::session &db, std::string_view table_name, rs::model::CModel auto &&m) {
    auto vs = std::apply([](auto&&... xs) { 
          return std::array{
              rs::if_else(xs.opt_value.has_value(),
                      [](auto &&f) { return fmt::format("'{}'", *f.opt_value); },
                      [](auto &&f) { return "NULL"; },
              xs)...};
    }, m.fields());

    db << fmt::format("INSERT INTO {} ({}) VALUES({})", table_name,
              fmt::join(m.field_names(), ","),
              fmt::join(std::move(vs), ","));
}

} // ns rs::actions

#endif // RS_ACTIONS_HPP
