#ifndef UTIL_HPP
#define UTIL_HPP

#include <variant>
#include <restinio/all.hpp>
#include <nlohmann/json.hpp>
#include <soci/soci.h>

template <typename... Fs>
struct overloaded : Fs... { using Fs::operator()...; };

template <typename... Fs> overloaded(Fs...) -> overloaded<Fs...>;

template <typename... Ts>
struct print_types;

namespace rs {

struct Shared_data {
    soci::session &db;
};

using json_t = nlohmann::json;

class HResponse {
public:
    json_t && extract_json() { return std::move(m_json); }
    virtual ~HResponse() = default;
protected:
    HResponse(json_t && json) : m_json(std::move(json)) { }
private:
    json_t m_json = nullptr;
};

class HSuccess : public HResponse {
public:
    HSuccess(json_t && json = nullptr) : HResponse(std::move(json)) { }
};

class HError : public HResponse {
public:
    enum class id { 
        Other,
        InvalidParams,
        JsonParseError,
    };

    HError(id err_id, json_t && json) : HResponse(std::move(json)), m_id(err_id) { }
    id error_id() { return m_id; }
private:
    id m_id = id::Other;
};

using router_t = restinio::router::express_router_t<>;
using handler_t = restinio::router::express_request_handler_t;
using api_response_t = std::variant<HSuccess, HError>;
using api_handler_t = api_response_t(*)(Shared_data&,json_t);
}

#endif
