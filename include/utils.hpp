#ifndef RS_UTILS_HPP
#define RS_UTILS_HPP

#include "typedefs.hpp"

namespace rs {
json_t success_response(std::string_view info = "") {
    json_t json;
    json["message"] = "SUCCESS";
    if (!info.empty())
        json["info"] = info;
    return json;
};
}

#endif 
