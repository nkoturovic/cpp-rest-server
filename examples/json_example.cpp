#include <iostream>
#include <nlohmann/json.hpp>
#include "models.hpp" // rs::User

int main()
{
    rs::model::User kotur {
        .username = { "kotur" },
        .password = { "pwd" },
        .email = { "" },
        .firstname = { "Nebojsa" },
        .lastname = { "Koturovic" },
        .born = { "1995-10-10" },
    };

    /* operator<< je automatski impl (refleksija) */
    std::cout << kotur << '\n';

    /* from_json, to_json su automatski impl. */
    std::cout << nlohmann::json(kotur).dump(2) << '\n';

    /* Izvlacenje OPISA nezadovoljenih CONSTRAINT-ova */
    if (auto ds_map = kotur.unsatisfied_constraints()
                           .transform(rs::cnstr::description); ds_map.size())
        std::cout << rs::json_t(ds_map).dump(2) << '\n';

    return 0;
}
