#include <iostream>
#include <concepts/concepts.hpp>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>
#include "model/model.hpp"

int main() {

   soci::session sql(soci::sqlite3, "dbname=db.sqlite");
   {
        rs::model::User user; // ovde cemo smestiti podatke
        soci::statement getUsersStmt = (sql.prepare << "SELECT * FROM users", soci::into(user));
        getUsersStmt.execute();

        // Izvlacimo opise svih nezadovoljenih constraint-ova i stampamo na cout
        if (auto unsat = rs::model::fmap_unsatisfied_cnstr(user, cnstr::name); unsat.size()) {
                std::cout << nlohmann::json(unsat) << '\n';
        }

        // izvlacimo sve usere dok ih ima
        while (getUsersStmt.fetch()) {
            user.id.erase_value(); // brisemo id (ne sme videti korisnik npr.)
            nlohmann::json j(user); // pretvaramo u json
            std::cout << j << '\n'; // stampamo
            // Pretvaramo u mapu i iteriramo [k,v]
            for (auto &[k,v] : rs::model::to_map(user)) {
                std::cout << k << " -> " << v << std::endl;
            }
            std::cout << "======================================\n";
        }

        // sql << "INSERT INTO users ()", soci::use(user);
    }

    return 0;
}
