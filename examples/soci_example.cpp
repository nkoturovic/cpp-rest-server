#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>

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
	auto print_field = [](const auto &field) {
		if (field.opt_value.has_value())
			std::cout << *field.opt_value << '\n';;
	};
	
	// izvlacimo sve usere dok ih ima
	while (getUsersStmt.fetch()) {
		std::cout << "Printing json:\n";
		user.id.opt_value.reset(); // brisemo id (ne sme videti korisnik npr.)
		std::cout << user.json().dump(2) << '\n';

		std::cout << "Printing values:\n";
		std::apply([&](auto&& ... xs) {
			(print_field(xs),...);
		}, user.fields());

		std::cout << "======================================\n";
	}

    return 0;
}
