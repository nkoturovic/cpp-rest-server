#include <iostream>
#include <nlohmann/json.hpp>
#include "model/models.hpp" // rs::User

int main()
{
    rs::model::User kotur {
        .username = { "kotur" },
        .password = { "pwd" },
        .email = { "" },
        .firstname = { "Nebojsa" },
        .lastname = { "Koturovic" },
        .born = { 813336633 }, /* Linux Time */
    };

    /* operator<< je automatski impl (refleksija) */
    std::cout << kotur << '\n';

    /* from_json, to_json su automatski impl. */
    std::cout << nlohmann::json(kotur).dump(2) << '\n';

    /* Izvlacenje OPISA nezadovoljenih CONSTRAINT-ova */
    if (auto ds_map = rs::model::apply_to_unsatisfied_cnstrs_of_model(kotur, cnstr::description, "rs"); ds_map.size())
        std::cout << rs::json_t(ds_map).dump(2) << '\n';

    return 0;
}
