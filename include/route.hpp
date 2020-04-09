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
    virtual restinio::request_handling_status_t do_handle(restinio::request_handle_t req, restinio::router::route_params_t parms) = 0;
public:
    virtual ~Route() = default;
    restinio::request_handling_status_t operator()(restinio::request_handle_t, restinio::router::route_params_t);
    const std::string& path() const;
    Method method() const;
};

class ApiRoute : public Route {
    api_handler_t m_api_handler = { nullptr };
    restinio::request_handling_status_t do_handle(restinio::request_handle_t, restinio::router::route_params_t) override;
    inline static Shared_data *s_data = nullptr;
public:
    static void initialize_shared_data(Shared_data * sd);
    ApiRoute(Method method, std::string path, api_handler_t json_handler_func);
    ~ApiRoute() = default;
};

}

#endif // ROUTE_HPP
