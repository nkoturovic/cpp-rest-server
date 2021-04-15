#include <iostream>
#include <nlohmann/json.hpp>
#include <models.hpp> // rs::User

#include <fmt/format.h>
#include <nlohmann/json.hpp>

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
    std::cout << kotur.json().dump(2) << '\n';

    /* Izvlacenje OPISA nezadovoljenih CONSTRAINT-ova */
    if (auto ds_map = kotur.get_unsatisfied_constraints()
                           .transform(rs::model::cnstr::get_description); ds_map.size())
        std::cout << nlohmann::json(ds_map).dump(2) << '\n';

    return 0;
}
