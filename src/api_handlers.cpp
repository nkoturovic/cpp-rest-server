#include "api_handlers.hpp"
#include "model/model.hpp"

namespace rs::api_handlers {

// api_response_t user_get(Shared_data &data, json_t json_request) {
//    // json_t j_user {
//    //     { "id", 0},
//    //     { "username", "kotur"},
//    //     { "firstname", "Nebojsa"},
//    //     { "lastname", "Koturovic"}
//    // };
// 
//    // try {
//    //     model::User u(j_user);
//    //     std::cout << "FROM Handler: " << u << std::endl;
//    //     return nlohmann::json(u);
//    // } catch (const std::exception &e) {
//    //     std::cout << "ERR FROM Handler: " << e.what() << std::endl;
//    // }
//    // //json_t jreturn(u); 
//    // return j_user;
//    //
//     std::cout << json_request << std::endl;
//     model::User ru(json_request);
//     std::cout << ru << std::endl;
// 
//     model::User user;
//     soci::statement getUsersStmt = (data.db.prepare << "SELECT * FROM users", soci::into(user));
//     std::cout << "1111ssss" << std::endl;
//     getUsersStmt.execute();
//     std::cout << "2222ssss" << std::endl;
//     //if (auto unsat = model::unsatisfied_constraints(user); unsat.size()) {
//     //        std::cout << nlohmann::json(unsat) << '\n';
//     //}
//     std::vector<json_t> users;
//     std::cout << "333333ssss" << std::endl;
//     while (getUsersStmt.fetch()) {
//         std::cout << user << std::endl;
//         users.push_back(user);
//     }
// 
//     std::cout << "SUCCRSSSSSSSSSSS" << std::endl;
// 
//     return users;
// }
// 

}
