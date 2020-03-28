#ifndef ROUTE_HPP
#define ROUTE_HPP

#include "typedefs.hpp"

namespace rs {

enum class Method { INVALID, GET, POST, DELETE, PUT };

class Route {
protected:
    Route(Method method, std::string path);
private:
    Method m_method { Method::INVALID };
    std::string m_path;
    virtual handler_t gen_callback_func_impl() const = 0;
public:
    virtual ~Route() = default;
    const std::string& path() const;
    Method method() const;
    handler_t gen_callback_func() const;
};

class ApiRoute : public Route {
    api_handler_t m_api_handler = { nullptr };

    handler_t gen_callback_func_impl() const override;
public:
    ApiRoute(Method method, std::string path, api_handler_t json_handler_func);
    ~ApiRoute() = default;
};

}

#endif // ROUTE_HPP
