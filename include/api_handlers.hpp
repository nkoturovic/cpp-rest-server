#include <nlohmann/json.hpp>

namespace rs::handlers {

// User
nlohmann::json user_get(std::optional<nlohmann::json> json_request);

}
