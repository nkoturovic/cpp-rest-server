#ifndef RS_ACTIONS_HPP
#define RS_ACTIONS_HPP

#include <sstream>
#include <map>
#include <boost/algorithm/string/join.hpp>

#include "errors.hpp"
#include "model/model.hpp"

namespace rs::actions {

std::vector<std::string> check_uniquenes_in_db(soci::session &db, std::string_view table_name, rs::model::CModel auto const& m) {
    auto us = m.get_unique_cnstr_fields();
    std::vector<std::string> duplicates;
    for (int count; auto &&[k,v] : us) {
         count = 0;
         db << "SELECT COUNT(*)" << " FROM " << table_name << " WHERE " << k << "=" << "\"" << v << "\"", soci::into(count);
         if (count) duplicates.push_back(std::move(k));
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
        models.push_back(std::move(m));
    }

    return models;
}

void insert_model_into_db(soci::session &db, std::string_view table_name, rs::model::CModel auto &&m) {
    auto [names, values] = m.fields_with_value_str();
    auto names_str = boost::algorithm::join(std::move(names), ", ");
    auto values_str = std::string{"'"}.append(boost::algorithm::join(std::move(values), "', '")).append("'");
    db << "INSERT INTO " << table_name << "(" << names_str << ")" << " VALUES(" <<  values_str << ")";
}

} // ns rs::actions

#endif // RS_ACTIONS_HPP
