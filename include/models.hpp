#ifndef RS_MODELS_HPP
#define RS_MODELS_HPP

#include "model/model.hpp"
#include "model/field.hpp"

namespace rs::model {

/* Models for Database */
struct User final : public Model<User> {
    Field<int32_t,cnstr::Unique> id;
    Field<std::string, cnstr::Unique, cnstr::Length<1,10>, cnstr::Required> username;
    Field<std::string, cnstr::Required, cnstr::Length<6,255>> password;
    Field<std::string, cnstr::Unique,cnstr::Required, cnstr::NotEmpty, cnstr::Length<2,32>> email;
    Field<std::string, cnstr::Required, cnstr::Length<2,64>> firstname;
    Field<std::string, cnstr::Required, cnstr::Length<2,64>> lastname;
    Field<std::string, cnstr::Required> born;
    Field<std::string, cnstr::Required> gender;
    Field<std::string> join_date;
    Field<int32_t> permission_group;
};

/* Request Parameters Models */
struct Id final : public Model<Id> {
    Field<int32_t> id;
}; 

} // ns rs::model

/* Required for REFLECTION!! */
/* Models for Database */

REFL_AUTO(
  type(rs::model::User),
  field(id),
  field(username),
  field(password),
  field(email),
  field(firstname),
  field(lastname),
  field(born),
  field(gender),
  field(join_date),
  field(permission_group)
)

/* Required to enable automatic reading from sql to model and from model to sql */
template <> struct soci::type_conversion<rs::model::User> : rs::model::specialize_model<rs::model::User> {};

/* Request Parameters Models */
REFL_AUTO(
  type(rs::model::Id),
  field(id)
)


#endif // RS_MODELS_HPP
