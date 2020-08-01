#ifndef RS_ERRORS_HPP
#define RS_ERRORS_HPP

#include "typedefs.hpp"
#include <string_view>

namespace rs {

/* Compile type concept (trait) for what is Error */
template<typename E>
concept CError = requires(E e) {
    { E::id } -> std::convertible_to<std::string_view>;
    { E::msg } -> std::convertible_to<std::string_view>;
};

struct OtherError {
     constexpr static std::string_view id = "OtherError";
     constexpr static std::string_view msg = "Other error";
     inline static const restinio::http_status_line_t status = restinio::status_internal_server_error();
};

struct InvalidParamsError {
     constexpr static std::string_view id = "InvalidParamsError";
     constexpr static std::string_view msg = "Invalid parameters";
     inline static const restinio::http_status_line_t status = restinio::status_bad_request();
};

struct JsonParseError {
     constexpr static std::string_view id = "JsonParseError";
     constexpr static std::string_view msg = "Error parsing JSON";
     inline static const restinio::http_status_line_t status = restinio::status_bad_request();
};

struct NotFoundError {
     constexpr static std::string_view id = "NotFoundError";
     constexpr static std::string_view msg = "Reqource not found";
     inline static const restinio::http_status_line_t status = restinio::status_not_found();
};

struct DBError {
     constexpr static std::string_view id = "NotFoundError";
     constexpr static std::string_view msg = "Reqource not found";
     inline static const restinio::http_status_line_t status = restinio::status_internal_server_error();
};

class Error final : std::exception {
public:
    using error_type = std::variant<OtherError, InvalidParamsError, JsonParseError, NotFoundError, DBError>;

    error_type error;
    nlohmann::json info = {};

    template <CError E>
    Error(E &&e, nlohmann::json &&info = {}) : error(std::move(e)), info(std::move(info)) {}

    std::string_view id() const { return std::visit([]<CError E>(E const&) { return E::id; }, this->error); }
    std::string_view msg() const { return std::visit([]<CError E>(E const&) { return E::msg; }, this->error); }
    const restinio::http_status_line_t status() const { return std::visit([]<CError E>(E const&) { return E::status; }, this->error); }

    constexpr const char * what() const noexcept override {
        return std::visit([]<CError E>(E const & /* */) {
                return E::msg.data();
        }, error);
    }
};

template <CError E> 
inline Error make_error(nlohmann::json &&j = {}) {
    return Error(E{}, std::move(j));
}

} // ns rs

template<rs::CError E>
struct nlohmann::adl_serializer<E> {
    static void to_json(nlohmann::json &j, const E& /**/) {
        j["error_id"] = E::id;
        j["message"] = E::msg;
   }
};

template <>
struct nlohmann::adl_serializer<rs::Error> {
    static void to_json(nlohmann::json &j, rs::Error const& err) {
        j = std::visit([]<rs::CError E>(E const& e) { return nlohmann::json(e); }, err.error);

        if (!err.info.is_null() || !err.info.empty())
            j["info"] = err.info;
    }
};

#endif 
