#ifndef RS_TYPEDEFS_HPP
#define RS_TYPEDEFS_HPP

#include <restinio/all.hpp>
#include <restinio/router/easy_parser_router.hpp>
#include <nlohmann/json.hpp>
#include <boost/lexical_cast.hpp>// for lexical_cast() 
#include "3rd_party/magic_enum.hpp"

/* Fix cast true -> 1 and false -> 0 */
namespace boost {
    template<> 
    bool lexical_cast<bool, std::string>(const std::string& arg) {
        std::istringstream ss(arg);
        bool b;
        ss >> std::boolalpha >> b;
        return b;
    }

    template<>
    std::string lexical_cast<std::string, bool>(const bool& b) {
        std::ostringstream ss;
        ss << std::boolalpha << b;
        return ss.str();
    }
}

namespace rs {

struct Unit {}; // Unit type -> composable replacement for void

namespace epr = restinio::router::easy_parser_router;
using router_t = restinio::router::easy_parser_router_t;

}

#endif
