#ifndef RS_ACTIONS_HPP
#define RS_ACTIONS_HPP

#include "model/constraint.hpp"
#include "composition.hpp"
#include "model/model_base.hpp"
#include "api_handlers.hpp"

namespace rs::actions {

template <rs::model::CModel M>
M check_constraints(const M &m) {

    //auto errs = model::fmap_unsatisfied_cnstr(m, []<cnstr::Cnstr C>() -> std::map<std::string, std::string> {
    //    return std::map<std::string, std::string> {
    //        { "name", cnstr::name.template operator()<C>() },
    //        { "desc", cnstr::description.template operator()<C>() }
    //    };
    //});

    auto errs = model::fmap_unsatisfied_cnstr(m, cnstr::description);

    if (errs.size())
        throw rs::ApiException(ApiErrorId::InvalidParams, errs);

    return m;
}

template <rs::model::CModel M>
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

template <rs::model::CModel M>
M from_request(json_t request) {
    return M(request);
}

template <rs::model::CModel M>
json_t from_model(M &&m) {
    return json_t(std::forward<M>(m));
}


template <rs::model::CModel M>
auto insert_model_into_db(soci::session &db, std::string table_name) {
    return rs::transform(from_request<M>)
         | rs::transform(check_constraints<M>)
         | rs::transform([&db, &table_name](M &&m) -> M { return check_uniquenes_in_db<M>(db, table_name, std::move(m)); })
         | rs::transform([&db, &table_name](M &&m) -> rs::json_t { 
                rs::api_handlers::insert_model_into_db(db, table_name, std::move(m));
                return nullptr;
         })
         ;
}

template <rs::model::CModel M>
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
