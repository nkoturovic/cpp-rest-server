# C++ REST Server 

## Dependencies

- c++20 compatible compiler (GCC/Clang)
- **fmt-lib** (restinio dependency): [https://github.com/fmtlib/fmt](https://github.com/fmtlib/fmt)
- **restinio** (HTTP Server + router): [https://github.com/Stiffstream/restinio](https://github.com/Stiffstream/restinio)
- **refl-cpp** (Static reflection): [https://github.com/veselink1/refl-cpp](https://github.com/veselink1/refl-cpp)
- **BoostHana** (metaprogramming): [https://www.boost.org/doc/libs/1_73_0/libs/hana/doc/html/index.html](https://www.boost.org/doc/libs/1_73_0/libs/hana/doc/html/index.html)
- **nlohmann::json** (json): [https://github.com/nlohmann/json](https://github.com/nlohmann/json)
- **SOCI** (DBAccessLib for SQL/sqlite): [https://github.com/SOCI/soci](https://github.com/SOCI/soci)

## What are goals of this application?

- Reducing boilerplate code and emracing DRY principle of programming.
- Making nice framework for building RESTful APIs.
- High performance, scalability and low latency

## What are some of the key features?

- Constraints on model are represented easily with type oriented constraint design.

```c++
/* Base class Model will inject json(), get_unsatisfied_constraints() and more similar methods */
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

```c++
rs::model::User kotur {
    .username = { "kotur" },
    .password = { "pwd" },
    .email = { "" },
    .firstname = { "Nebojsa" },
    .lastname = { "Koturovic" },
    .born = { "1995-10-10" },
};

/* We get '.json()' method for free with zero coding */
std::cout << kotur.json().dump(2);

/* Validation: Geting description map<key,vec<dsc>> for all unsatisfied constraints */
if (auto dsc_map = kotur.get_unsatisfied_constraints()
                        .transform(rs::model::cnstr::get_description); !dsc_map.empty())
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

Also, our models are automatically adjusted to work with SOCI-sql library for free.

```c++
rs::model::User user;
soci::statement getUsersStmt = (sql.prepare << "SELECT * FROM users", soci::into(user));
getUsersStmt.execute();

while (getUsersStmt.fetch())
    std::cout << user.json() << '\n';
```

**Output**:

```json
{
  "born": "1994-04-20",
  "email": "mladen@gmail.com",
  "firstname": "Mladen",
  "gender": "m",
  "id": 1,
  "join_date": "2020-06-30",
  "lastname": "Mladenovic",
  "password": "qweqwe123",
  "permission_group": 3,
  "username": "djomla96"
}
{
  "born": "1995-10-10",
  "email": "necer95@gmail.com",
  "firstname": "Nebojsa",
  "gender": "m",
  "id": 2,
  "join_date": "2020-07-01",
  "lastname": "Koturovic",
  "password": "qweqwe123",
  "permission_group": 3,
  "username": "kotur"
}
...
```

## Using and building APP

### Requirements

- A compiler with c++20 support
- [CMake build tool](https://cmake.org)
- [Conan package manager](https://conan.io/)
- [Perl scripting language](https://www.perl.org/) (One of the dependencies require it for generating Makefiles)

### Building

```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j8
```

### Running

```
cd ./build/bin/
./rest-server
```
