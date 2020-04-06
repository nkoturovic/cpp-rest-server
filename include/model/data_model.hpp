#include "data_model_base.hpp"

namespace model {

struct User : Model {
    Integer<> id;
    Text<cnstr::Length<1,10>> username;
    Text<> password;
    Text<> email;
    Text<> firstname;
    Text<> lastname;
    Text<> status;
};

}
