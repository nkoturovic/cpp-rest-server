#include <iostream>
#include <concepts/concepts.hpp>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>
#include "model/model.hpp"

int main() {

   soci::session sql(soci::sqlite3, "dbname=ImageShare.sqlite");
   {
        model::User user;
        soci::statement getUsersStmt = (sql.prepare << "SELECT * FROM users", soci::into(user));
        getUsersStmt.execute();

        if (auto unsat = model::unsatisfied_constraints(user); unsat.size()) {
                std::cout << nlohmann::json(unsat) << '\n';
        }

        while (getUsersStmt.fetch()) {
            user.id.erase_value();
            nlohmann::json j(user);
            std::cout << j << '\n';
            for (auto &[k,v] : model::to_map(user)) {
                std::cout << k << " -> " << v << std::endl;
            }
        }

        // sql << "INSERT INTO users ()", soci::use(user);
    }

    return 0;
}
