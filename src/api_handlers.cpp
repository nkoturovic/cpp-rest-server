#include "api_handlers.hpp"
#include "model/model.hpp"

namespace rs::handlers {

namespace user {

api_handler_response_t user_get(std::optional<json_t> json_request) {
    json_t j_user {
        { "id", 0},
        { "username", "kotur"},
        { "firstname", "Nebojsa"},
        { "lastname", "Koturovic"}
    };

    try {
        model::User u(j_user);
        std::cout << "FROM Handler: " << u << std::endl;
        return nlohmann::json(u);
    } catch (const std::exception &e) {
        std::cout << "ERR FROM Handler: " << e.what() << std::endl;
    }
    //json_t jreturn(u); 
    return j_user;
}

}

}
