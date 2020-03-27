#include <nlohmann/json.hpp>
#include "api_handlers.hpp"

namespace rs::handlers {

// User
nlohmann::json user_get(std::optional<nlohmann::json> json_request) {
    return { {"message", "hello world!"} };
}

}
