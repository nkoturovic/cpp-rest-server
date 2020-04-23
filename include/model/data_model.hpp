#include "data_model_base.hpp"

namespace model {

struct User : Model {
    Integer<> id;
    Text<cnstr::Length<1,10>> username;
    Text<cnstr::Required> password;
    Text<cnstr::Required> email;
    Text<cnstr::Required> firstname;
    Text<cnstr::Required> lastname;
    Integer<cnstr::Required> born;
    Text<> status;
};

}
