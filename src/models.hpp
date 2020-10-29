#ifndef RS_MODELS_HPP
#define RS_MODELS_HPP

#include "model/model.hpp"
#include "model/field.hpp"

namespace rs::model {

/* Models for Database */
struct User final : Model<User> {
    Field<int32_t, cnstr::Unique> id;
    Field<std::string, cnstr::Unique, cnstr::Length<1,20>, cnstr::Required> username;
    Field<std::string, cnstr::Required, cnstr::ValidPassword > password;
    Field<std::string, cnstr::Unique, cnstr::Required, cnstr::ValidEmail> email;
    Field<std::string, cnstr::Length<2,64>> firstname;
    Field<std::string, cnstr::Length<2,64>> lastname;
    Field<std::string, cnstr::ISOdate > born;
    Field<std::string, cnstr::ValidGender> gender;
    Field<std::string, cnstr::Length<0,8192>> biography;
    Field<std::string, cnstr::ISOdate> join_date;
    Field<int32_t> permission_group;
};

/* Models for Database */
struct Photo final : Model<Photo> {
    Field<int32_t, cnstr::Unique> id;
    Field<std::string, cnstr::Required, cnstr::ValidImageExtension> extension;
    Field<std::string, cnstr::Length<1,255>, cnstr::Required> title;
    Field<std::string, cnstr::Length<0,255>, cnstr::Required, cnstr::ValidCategory> category;
    Field<std::string, cnstr::Length<0,4096>> description;
    Field<int32_t,cnstr::Unique> uploaded_by;
    Field<std::string> upload_time;
    Field<int32_t, cnstr::Required, cnstr::Between<0,1>> is_private;
};

/* Request Parameters Models */
struct RefreshToken final : Model<RefreshToken> {
    Field<std::string> refresh_token;
}; 

struct AuthToken final : Model<AuthToken> {
    Field<std::string> auth_token;
}; 

struct RefreshAndAuthTokens final : Model<RefreshAndAuthTokens> {
    Field<std::string> refresh_token;
    Field<std::string> auth_token;
}; 


/* Models for Database */
struct UserCredentials final : Model<UserCredentials> {
    Field<std::string, cnstr::Unique, cnstr::Length<1,20>, cnstr::Required> username;
    Field<std::string, cnstr::Required, cnstr::ValidPassword > password;
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
  field(biography),
  field(join_date),
  field(permission_group)
)

REFL_AUTO(
    type(rs::model::Photo),
    field(id),
    field(extension),
    field(title),
    field(category),
    field(description),
    field(uploaded_by),
    field(upload_time),
    field(is_private)
)

/* Request Parameters Models */
REFL_AUTO(
  type(rs::model::RefreshToken),
  field(refresh_token)
)

REFL_AUTO(
  type(rs::model::AuthToken),
  field(auth_token)
)

REFL_AUTO(
  type(rs::model::RefreshAndAuthTokens),
  field(refresh_token),
  field(auth_token)
)

REFL_AUTO(
  type(rs::model::UserCredentials),
  field(username),
  field(password)
)

#endif // RS_MODELS_HPP
