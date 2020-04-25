#ifndef RS_DATA_MODEL_HPP
#define RS_DATA_MODEL_HPP

#include "data_model_base.hpp"

namespace rs::model {

struct User : Model {
    Integer<> id;
    Text<cnstr::Unique,cnstr::Length<1,10>, cnstr::Required> username;
    Text<cnstr::Required> password;
    Text<cnstr::Unique,cnstr::Required> email;
    Text<cnstr::Required> firstname;
    Text<cnstr::Required> lastname;
    Integer<cnstr::Required> born;
    Text<> status;
};

} // ns rs::model

#endif // RS_DATA_MODEL_HPP
