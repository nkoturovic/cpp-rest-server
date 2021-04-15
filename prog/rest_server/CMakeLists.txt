set(REST_SERVER_HEADERS 
    actions.hpp errors.hpp handler.hpp models.hpp routes.hpp utils.hpp 
    model/field.hpp model/constraint.hpp model/model.hpp
)

set(REST_SERVER_SRCS main.cpp)
add_executable(prog_rest_server ${REST_SERVER_SRCS} ${REST_SERVER_HEADERS})

target_include_directories(prog_rest_server SYSTEM PRIVATE 3rd_party)
target_include_directories(prog_rest_server PRIVATE ./)
target_link_libraries(prog_rest_server project_options project_warnings ${CONAN_LIBS})
