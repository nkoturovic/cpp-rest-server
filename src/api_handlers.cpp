#include "api_handlers.hpp"

namespace rs::handlers {

namespace user {

api_handler_response_t user_get(std::optional<json_t> json_request) {
    return json_t{ {"message", "hello world!"} };
}

}

}
