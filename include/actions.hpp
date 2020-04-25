#ifndef ACTIONS_HPP
#define ACTIONS_HPP

#include "model/constraint.hpp"
#include "composition.hpp"
#include "model/data_model_base.hpp"
#include "api_handlers.hpp"

namespace rs::actions {

template <typename M> requires concepts::derived_from<M,model::Model>
M check_constraints(const M &m) {
    if (auto vec = model::unsatisfied_constraints(m); vec.size())
        throw rs::ApiException(ApiErrorId::InvalidParams, vec);
    return m;
}

template <typename M> requires concepts::derived_from<M,model::Model>
M check_uniquenes_in_db(soci::session &db, std::string table_name, const M &m) {
    auto us = model::unique_cnstr_fields(m);
    for (const auto &[k,v] : us) {
         int count = 0;
         db << "SELECT COUNT(*)" << " FROM " << table_name << " WHERE " << k << "=" << "\"" << v << "\"", soci::into(count);
         if (count) throw ApiException(ApiErrorId::InvalidParams, json_t{ { k, std::vector{ "Already exist in db" } } });
    }
    return m;
}

// auto check_auth() {
//     // ...
// }

template <typename M> requires concepts::derived_from<M,model::Model>
M from_request(json_t request) {
    return M(request);
}

template <typename M> requires concepts::derived_from<M,model::Model>
json_t from_model(M &&m) {
    return json_t(std::forward<M>(m));
}


template <typename M> requires concepts::derived_from<M,model::Model>
auto insert_model_into_db(soci::session &db, std::string table_name) {
    return rs::transform(from_request<M>)
         | rs::transform(check_constraints<M>)
         | rs::transform([&db, &table_name](M &&m) -> M { return check_uniquenes_in_db<M>(db, table_name, std::move(m)); })
         | rs::transform([&db, &table_name](M &&m) -> rs::json_t { 
                return rs::api_handlers::insert_model_into_db(db, table_name, std::move(m));
         })
         ;
}

template <typename M> requires concepts::derived_from<M,model::Model>
auto get_models_from_db(soci::session &db, std::string table_name) {
  return rs::transform([&db, &table_name](json_t) -> std::vector<M> { 
                return rs::api_handlers::get_models_from_db<M>(db, table_name);
         })
         | rs::transform([](std::vector<M> &&v) { 
                 return json_t(std::move(v));
         })
         ;
}

}

#endif
