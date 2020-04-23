#include "data_model.hpp"

REFL_AUTO(
  type(model::User),
  field(id),
  field(username),
  field(password),
  field(email),
  field(firstname),
  field(lastname),
  field(born),
  field(status)
)

template <> struct soci::type_conversion<model::User> : model::specialize_model<model::User> {};
