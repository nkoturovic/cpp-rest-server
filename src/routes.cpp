#include "routes.hpp"
#include "api_handlers.hpp"

#define API_PREFIX R"(/api/)"
#define concat(first, second) first second

using namespace restinio; 

namespace rs {

std::vector<std::shared_ptr<Route>> get_routes() {
    return {
        make_shared<ApiRoute>(Method::GET, concat(API_PREFIX, R"(user)"), handlers::user_get)
    };
}

}
