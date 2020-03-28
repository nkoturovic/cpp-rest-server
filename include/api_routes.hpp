#ifndef API_ROUTES_HPP
#define API_ROUTES_HPP

#include "route.hpp"
#include "api_handlers.hpp"

#define API_PREFIX R"(/api/)"
#define concat(first, second) first second

namespace rs {

inline std::vector<std::shared_ptr<Route>> get_api_routes() {
    return {
        make_shared<ApiRoute>(Method::GET, concat(API_PREFIX, R"(user)"), handlers::user::user_get)
    };
}
}

#undef API_PREFIX
#undef concat
#endif // API_ROUTES_HPP
