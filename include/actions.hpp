#ifndef RS_ACTIONS_HPP
#define RS_ACTIONS_HPP

#include <sstream>
#include <map>
#include <boost/algorithm/string/join.hpp>

#include "typedefs.hpp"
#include "errors.hpp"
#include "model/model.hpp"

namespace rs::actions {

template <rs::model::CModel M>
std::vector<std::string> check_uniquenes_in_db(soci::session &db, std::string_view table_name, const M &m) {
    auto us = model::unique_cnstr_fields(m);
    std::vector<std::string> duplicates;
    for (const auto &[k,v] : us) {
         int count = 0;
         db << "SELECT COUNT(*)" << " FROM " << table_name << " WHERE " << k << "=" << "\"" << v << "\"", soci::into(count);
         if (count) duplicates.push_back(k);
    }
    return duplicates;
}

template <rs::model::CModel M>
std::vector<M> get_models_from_db(soci::session &db, std::string_view table_name, std::string_view filter = "") {
    M m;
    std::string filter_stmt = "";

    if (filter.size())
        filter_stmt.append(" WHERE ").append(filter);

    soci::statement get_models_stmt = (db.prepare << "SELECT * FROM " << table_name << filter_stmt, soci::into(m));
    get_models_stmt.execute();

    std::vector<M> models;

    while (get_models_stmt.fetch()) {
        models.push_back(m);
    }

    return models;
}

template <rs::model::CModel M>
void insert_model_into_db(soci::session &db, std::string_view table_name, M && m) {
    auto [names, values] = initialized_fields_str(m);
    auto names_str = boost::algorithm::join(std::move(names), ", ");
    auto values_str = std::string{"\""}.append(boost::algorithm::join(std::move(values), "\", \"")).append("\"");

    db << "INSERT INTO " << table_name << "(" << names_str << ")" << " VALUES(" <<  values_str << ")";
}

}

#endif // RS_ACTIONS_HPP
