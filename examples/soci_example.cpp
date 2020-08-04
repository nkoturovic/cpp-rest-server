#include <iostream>
#include <concepts/concepts.hpp>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>
#include <fmt/format.h>

#include "models.hpp"

int main() 
{
    soci::session sql(soci::sqlite3, "dbname=db.sqlite");
    rs::model::User user; // ovde cemo smestiti podatke
    soci::statement getUsersStmt = (sql.prepare << "SELECT * FROM users", soci::into(user));
    getUsersStmt.execute();

    // Izvlacimo opise svih nezadovoljenih constraint-ova i stampamo na cout
    if (auto unsat = user.get_unsatisfied_constraints()
                         .transform(rs::model::cnstr::get_name); unsat.size()) {
            std::cout << nlohmann::json(unsat) << '\n';
    }

    // izvlacimo sve usere dok ih ima
    while (getUsersStmt.fetch()) {
        user.id.erase_value(); // brisemo id (ne sme videti korisnik npr.)
        nlohmann::json j(user); // pretvaramo u json
        std::cout << j << '\n'; // stampamo
        // Pretvaramo u mapu i iteriramo [k,v]
        auto [ks,vs] = user.fields_with_value_str();
        for (unsigned i = 0; i < ks.size(); i++) {
            std::cout << ks[i] << " -> " << vs[i] << std::endl;
        }
        std::cout << "======================================\n";
    }

    return 0;
}
