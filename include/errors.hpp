#ifndef RS_ERRORS_HPP
#define RS_ERRORS_HPP

#include "typedefs.hpp"
#include <string_view>

namespace rs {

/* Compile type concept (trait) for what is Error */
template<typename E>
concept CError = requires(E e) {
    { e.what() } -> std::convertible_to<const char *>;
    { e.id() } -> std::convertible_to<std::string_view>;
    { e.msg() } -> std::convertible_to<std::string_view>;
    { e.status() } -> std::convertible_to<restinio::http_status_line_t>;
};

struct Error : std::exception {
    nlohmann::json info;

    Error(nlohmann::json info = {}) : info(info) {} 

    constexpr const char * what() const noexcept override {
        return this->msg().data();
    }
    virtual constexpr std::string_view id() const = 0;
    virtual constexpr std::string_view msg() const = 0;
    virtual restinio::http_status_line_t status() const = 0;
    nlohmann::json json() const {
        nlohmann::json j;
        j["error_id"] = this->id();
        j["message"] = this->msg();

        if (!this->info.is_null() || !this->info.empty())
            j["info"] = this->info;
        return j;
    }
protected:
    virtual ~Error() = default;
};

struct OtherError final : Error {
    using Error::Error;
    constexpr std::string_view id() const override { return "OtherError"; }
    constexpr std::string_view msg() const override { return "Other error"; }
    inline restinio::http_status_line_t status() const override { return restinio::status_internal_server_error(); }
};

struct InvalidParamsError final : Error {
    using Error::Error;
    constexpr std::string_view id() const override { return "InvalidParamsError"; }
    constexpr std::string_view msg() const override { return "Invalid Parameters"; }
    inline restinio::http_status_line_t status() const override { return restinio::status_bad_request(); }
};

struct JsonParseError final : Error {
    using Error::Error;
    constexpr std::string_view id() const override { return "JsonParseError"; }
    constexpr std::string_view msg() const override { return "Error parsing JSON"; }
    inline restinio::http_status_line_t status() const override { return restinio::status_bad_request(); }
};

struct NotFoundError final : Error {
    using Error::Error;
    constexpr std::string_view id() const override { return "NotFoundError"; }
    constexpr std::string_view msg() const override { return "Reource not found"; }
    inline restinio::http_status_line_t status() const override { return restinio::status_not_found(); }
};

struct DBError final : Error {
    using Error::Error;
    constexpr std::string_view id() const override { return "DBError"; }
    constexpr std::string_view msg() const override { return "Database error"; }
    inline restinio::http_status_line_t status() const override { return restinio::status_bad_request(); }
};

} // ns rs

template<rs::CError E>
struct nlohmann::adl_serializer<E> {
    static void to_json(nlohmann::json &j, const E& e) {
        j = e.json();
   }
};

#endif 
