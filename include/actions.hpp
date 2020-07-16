#ifndef RS_ACTIONS_HPP
#define RS_ACTIONS_HPP

#include "typedefs.hpp"
#include "model/model.hpp"
#include <sstream>
#include <map>

namespace rs::actions {

template <rs::model::CModel M>
constexpr M check_uniquenes_in_db(soci::session &db, std::string_view table_name, const M &m) {
    auto us = model::unique_cnstr_fields(m);
    for (const auto &[k,v] : us) {
         int count = 0;
         db << "SELECT COUNT(*)" << " FROM " << table_name << " WHERE " << k << "=" << "\"" << v << "\"", soci::into(count);
         if (count) throw ApiException(ApiErrorId::InvalidParams, json_t{ { k, std::vector{ "Already exist in db" } } });
    }
    return m;
}

template <rs::model::CModel M>
std::vector<M> get_models_from_db(soci::session &db, std::string_view table_name, std::string_view filter = "") {
    M m;

    std::string filterStmt = "";

    if (filter.size())
        filterStmt.append(" WHERE ").append(filter);

    soci::statement getModelsStmt = (db.prepare << "SELECT * FROM " << table_name << filterStmt, soci::into(m));
    getModelsStmt.execute();

    std::vector<M> models;

    while (getModelsStmt.fetch()) {
        models.push_back(m);
    }

    return models;
}

template <rs::model::CModel M>
void insert_model_into_db(soci::session &db, std::string_view table_name, M && m) {
    auto model_map = model::to_map(m);
    std::vector<std::string> keys (model_map.size());
    std::vector<std::string> values (model_map.size());

    std::transform(model_map.begin(), model_map.end(), keys.begin(), [](const auto &m) { return m.first; });
    std::transform(model_map.begin(), model_map.end(), values.begin(), [](const auto &m) { return m.second; });

    std::string ks = std::accumulate(keys.begin(), keys.end(), std::string{}, [](std::string s1, std::string s2) { 
            return std::move(s1) + (s1 != "" ? ", " : "") + std::move(s2);
    });

    std::string vs = std::accumulate(values.begin(), values.end(), std::string{}, [](std::string s1, std::string s2) { 
            return std::move(s1) + (s1 != "" ? ", " : "")  + "\"" + std::move(s2) + "\"";
    });

    db << "INSERT INTO " << table_name << "(" << ks << ")" << " VALUES(" << vs << ")";
}

}

#endif // RS_ACTIONS_HPP
