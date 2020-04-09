#include "api_handlers.hpp"
#include "model/model.hpp"

namespace rs::api_handlers {

api_handler_response_t user_get(Shared_data &data, std::optional<json_t> json_request) {
   // json_t j_user {
   //     { "id", 0},
   //     { "username", "kotur"},
   //     { "firstname", "Nebojsa"},
   //     { "lastname", "Koturovic"}
   // };

   // try {
   //     model::User u(j_user);
   //     std::cout << "FROM Handler: " << u << std::endl;
   //     return nlohmann::json(u);
   // } catch (const std::exception &e) {
   //     std::cout << "ERR FROM Handler: " << e.what() << std::endl;
   // }
   // //json_t jreturn(u); 
   // return j_user;
   //
    model::User user;
    soci::statement getUsersStmt = (data.db.prepare << "SELECT * FROM users", soci::into(user));
    getUsersStmt.execute();
    //if (auto unsat = model::unsatisfied_constraints(user); unsat.size()) {
    //        std::cout << nlohmann::json(unsat) << '\n';
    //}
    std::vector<json_t> users;
    while (getUsersStmt.fetch()) {
        users.push_back(user);
    }

    return users;
}

}
