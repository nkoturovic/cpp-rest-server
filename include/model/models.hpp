#ifndef RS_MODELS_HPP
#define RS_MODELS_HPP

#include "model.hpp"
#include "field.hpp"

namespace rs::model {
struct User : Model {
    Field<int,cnstr::Unique> id;
    Field<std::string, cnstr::Unique, cnstr::Length<1,10>, cnstr::Required> username;
    Field<std::string, cnstr::Required, cnstr::Length<6,255>> password;
    Field<std::string, cnstr::Unique,cnstr::Required, cnstr::NotEmpty, cnstr::Length<2,32>> email;
    Field<std::string, cnstr::Required, cnstr::Length<2,64>> firstname;
    Field<std::string, cnstr::Required, cnstr::Length<2,64>> lastname;
    Field<int, cnstr::Required> born;
    Field<std::string> status;
};
}

/* Required for REFLECTION!! */
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

/* Required to enable automatic reading from sql to model and from model to sql */
template <> struct soci::type_conversion<rs::model::User> : rs::model::specialize_model<rs::model::User> {};

#endif // RS_MODELS_HPP
