#ifndef RS_HANDLER_PARAMS_HPP
#define RS_HANDLER_PARAMS_HPP

#include "model/model.hpp"
#include "model/field.hpp"

namespace rs::handler_params {
/* Query parameters for get_user */
struct HPars_get_model_by_id : rs::model::Model {
    Field<unsigned long> id;
}; 
}

REFL_AUTO(
  type(rs::handler_params::HPars_get_model_by_id),
  field(id)
)

#endif
