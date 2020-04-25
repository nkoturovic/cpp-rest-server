#ifndef MODEL_HPP
#define MODEL_HPP

#include "data_model.hpp"

REFL_AUTO(
  type(rs::model::User),
  field(id),
  field(username),
  field(password),
  field(email),
  field(firstname),
  field(lastname),
  field(born),
  field(status)
)

template <> struct soci::type_conversion<rs::model::User> : rs::model::specialize_model<rs::model::User> {};

#endif // MODEL_HPP
