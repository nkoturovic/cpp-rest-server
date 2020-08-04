#ifndef RS_ERRORS_HPP
#define RS_ERRORS_HPP

#include <string_view>
#include <concepts>
#include <nlohmann/json.hpp>

#include <restinio/http_headers.hpp>

namespace rs {

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

/* Compile type concept (trait) for what is Error */
template<typename E>
concept CError = std::same_as<E, Error> || std::derived_from<E, Error>;

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
