#include <restinio/all.hpp>
#include <string_view>
#include <nlohmann/json.hpp>

namespace rs {

enum class Method { INVALID, GET, POST, DELETE, PUT };

class Route {
protected:
    using handler_type = restinio::router::express_request_handler_t;
    Route(Method method, std::string path) : m_method(method), m_path(std::move(path)) { }
private:
    Method m_method { Method::INVALID };
    std::string m_path;
    virtual handler_type gen_callback_func_impl() const = 0;
public:
    virtual ~Route() = default;
    const std::string& path() const { return m_path; }
    Method method() const { return m_method; }
    handler_type gen_callback_func() const { return gen_callback_func_impl(); /* from subclass */ }
};

class ApiRoute : public Route {
    using Route::handler_type;
    using json_handler_t = nlohmann::json(*)(std::optional<nlohmann::json>);
    json_handler_t m_json_handler = { nullptr };

    handler_type gen_callback_func_impl() const override {
        return [this](auto req, auto res) -> auto {
            return req->create_response()
            // set nullopt to actual data
           .set_body(m_json_handler({ std::nullopt }).dump())
           .append_header( restinio::http_field::content_type, "application/json" )
           .done();
        };
    }
public:
    ApiRoute(Method method, std::string path, json_handler_t json_handler_func) :
        Route(method, std::move(path)), m_json_handler(std::move(json_handler_func)) {}
    ~ApiRoute() = default;
};

std::vector<std::shared_ptr<Route>> get_routes();

}
