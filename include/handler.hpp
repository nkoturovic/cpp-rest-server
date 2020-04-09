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
    api_handler_t m_api_handler = { nullptr };
    restinio::request_handling_status_t do_handle(restinio::request_handle_t, restinio::router::route_params_t) override;
    inline static Shared_data *s_data = nullptr;
public:
    static void initialize_shared_data(Shared_data * sd);
    ApiHandler(api_handler_t json_handler_func);
    ~ApiHandler() = default;
};

}

#endif // HANDLER_HPP
