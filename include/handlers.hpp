#ifndef RS_HANDLERS_HPP
#define RS_HANDLERS_HPP

#include "handler_params.hpp"
#include "model/constraint.hpp"
#include "composition.hpp"
#include "actions.hpp"

namespace rs::helper {

template <rs::model::CModel M>
M to_model(json_t request) {
    return M(request);
}

template <typename T>
json_t to_json(T &&m) {
    return json_t(std::forward<T>(m));
}

template <rs::model::CModel M>
constexpr M check_constraints(const M &m) {

    //auto errs = model::apply_to_unsatisfied_cnstrs_of_model(m, []<cnstr::Cnstr C>() 
    // -> std::map<std::string, std::string> {
    //    return std::map<std::string, std::string> {
    //        { "name", cnstr::name.template operator()<C>() },
    //        { "desc", cnstr::description.template operator()<C>() }
    //    };
    //});

    auto errs = model::apply_to_unsatisfied_cnstrs_of_model(m, cnstr::description);

    if (errs.size())
        throw rs::ApiException(ApiErrorId::InvalidParams, errs);

    return m;
}

}

namespace rs::handlers {

template <rs::model::CModel M>
constexpr auto insert_model_into_db(soci::session &db, std::string_view table_name) {
    return rs::transform(helper::check_constraints<M>)
         | rs::transform([&db, table_name](M &&m) -> M { return actions::check_uniquenes_in_db<M>(db, table_name, std::move(m)); })
         | rs::transform([&db, table_name](M &&m) -> rs::json_t { 
                rs::actions::insert_model_into_db(db, std::move(table_name), std::move(m));
                return success_response("Model successfully inserted into db"); 
         })
         ;
}


template <rs::model::CModel M>
constexpr auto get_models_from_db(soci::session &db, std::string_view table_name, std::string_view filter = "") {
      return rs::transform([&db, table_name, filter](rs::unit) -> std::vector<M> { 
                return rs::actions::get_models_from_db<M>(db, table_name, filter);
       })
       | rs::transform(helper::to_json<std::vector<M>>)
       ;
}

auto get_user_by_id(soci::session &db) {
      return rs::transform([&db](handler_params::HPars_get_model_by_id pars, uint64_t id) -> model::User { 
            if (pars.id.has_value())
                std::cout << pars.id.value() << std::endl;

            auto vec = rs::actions::get_models_from_db<model::User>(db, "users", fmt::format("id = {}", id));
            if (vec.empty())
                throw ApiException(ApiErrorId::NotFound, "User with that id is not found");
            else 
                return vec.back();
      })
      | rs::transform(helper::to_json<model::User>)
      ;
}

// TODO: auto check_auth();
} 



#endif
