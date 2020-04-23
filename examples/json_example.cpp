#include <nlohmann/json.hpp>
#include <iostream>


int main()
{

    nlohmann::json j;
    std::cout << j.is_null() << std::endl;

    return 0;
}
