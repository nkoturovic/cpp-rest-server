#ifndef HANDLER_HPP
#define HANDLER_HPP

#include "typedefs.hpp"

namespace rs {

class Handler {
protected:
    Handler() = default;
private:
    virtual restinio::request_handling_status_t do_handle(restinio::request_handle_t req, restinio::router::route_params_t parms) = 0;
public:
    virtual ~Handler() = default;
    restinio::request_handling_status_t operator()(restinio::request_handle_t, restinio::router::route_params_t);
};

class ApiHandler : public Handler {
    api_handler_t m_api_handler;
    restinio::request_handling_status_t do_handle(restinio::request_handle_t, restinio::router::route_params_t) override;
public:
    
    ApiHandler(api_handler_t func) : m_api_handler(func) { }
    ~ApiHandler() = default;
};

}

#endif // HANDLER_HPP
