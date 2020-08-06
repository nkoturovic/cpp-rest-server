# C++ REST Server 

## Dependencies

- c++20 compatible compiler (GCC/Clang)
- **fmt-lib** (restinio dependency): [https://github.com/fmtlib/fmt](https://github.com/fmtlib/fmt)
- **restinio** (HTTP Server + router): [https://github.com/Stiffstream/restinio](https://github.com/Stiffstream/restinio)
- **refl-cpp** (Static reflection): [https://github.com/veselink1/refl-cpp](https://github.com/veselink1/refl-cpp)
- **BoostHana** (metaprogramming): [https://www.boost.org/doc/libs/1_73_0/libs/hana/doc/html/index.html](https://www.boost.org/doc/libs/1_73_0/libs/hana/doc/html/index.html)
- **nlohmann::json** (json): [https://github.com/nlohmann/json](https://github.com/nlohmann/json)
- **SOCI** (DBAccessLib for SQL/sqlite): [https://github.com/SOCI/soci](https://github.com/SOCI/soci)

## Way of representing models

Constraints on model are represented easily with type oriented constraint model.

```c++
struct User : Model<User> {
    Field<int,cnstr::Unique> id;
    Field<std::string, cnstr::Unique, cnstr::Length<1,10>, cnstr::Required> username;
    Field<std::string, cnstr::Required, cnstr::Length<6,255>> password;
    Field<std::string, cnstr::Unique,cnstr::Required, cnstr::NotEmpty, cnstr::Length<2,32>> email;
    Field<std::string, cnstr::Required, cnstr::Length<2,64>> firstname;
    Field<std::string, cnstr::Required, cnstr::Length<2,64>> lastname;
    Field<int, cnstr::Required> born; // linux time (since epoch)
    Field<std::string> status;
};

/* Required for static reflection */
REFL_AUTO(
  type(rs::model::User),
  field(id),
  field(username),
  field(password),
  ...
)
```

Json and SQL methods will be generated automatically through CRTP inheritance of base Model class.

```c++
rs::model::User kotur {
    .username = { "kotur" },
    .password = { "pwd" },
    .email = { "" },
    .firstname = { "Nebojsa" },
    .lastname = { "Koturovic" },
    .born = { "1995-10-10" },
};

/* We get '.json()' method for free with zero boilerplate */
std::cout << kotur.json().dump();

/* Geting description map(key,vec<dsc>) for all unsatisfied constraints if there is such */
if (auto dsc_map = kotur.get_unsatisfied_constraints()
                        .transform(rs::model::cnstr::get_description); dsc_map.size())
    std::cout << nlohmann::json(dsc_map).dump(2) << '\n';
```

**Output**:

```json
{
  "born": "1995-10-10",
  "email": "",
  "firstname": "Nebojsa",
  "lastname": "Koturovic",
  "password": "pwd",
  "username": "kotur"
}

{
  "email": [
    "Field must not be empty",
    "Length should be from 2 to 32"
  ],
  "gender": [
    "Field should not be empty or invalid"
  ],
  "password": [
    "Length should be from 6 to 255"
  ]
}
```
