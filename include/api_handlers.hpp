#ifndef API_HANDLERS_HPP
#define API_HANDLERS_HPP

#include "typedefs.hpp"
#include "model/model.hpp"
#include <concepts/concepts.hpp>
#include <sstream>
#include <map>

namespace rs::api_handlers {

template <typename M> requires concepts::derived_from<M,rs::model::Model>
std::vector<M> get_models_from_db(soci::session &db, std::string table_name) {
        M m;
        soci::statement getUsersStmt = (db.prepare << "SELECT * FROM " << table_name, soci::into(m));
        getUsersStmt.execute();
        std::vector<M> models;

        while (getUsersStmt.fetch()) {
            models.push_back(m);
        }
 
        return models;
 }

template <typename M> requires concepts::derived_from<M,rs::model::Model>
api_response_t insert_model_into_db(soci::session &db, std::string table_name, M && m) {
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

    return {}; // success
}

}

#endif // API_HANDLERS_HPP
