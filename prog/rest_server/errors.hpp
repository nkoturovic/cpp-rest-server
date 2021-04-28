#ifndef RS_ERRORS_HPP
#define RS_ERRORS_HPP

#include <string_view>
#include <concepts>
#include <nlohmann/json.hpp>

#include <restinio/http_headers.hpp>

namespace rs {

struct Error : std::exception {
    nlohmann::json info;

    explicit Error(nlohmann::json &&arg_info = {}) : info(std::move(arg_info)) {}

    [[nodiscard]] constexpr const char * what() const noexcept override {
        return this->msg().data();
    }
    virtual constexpr std::string_view id() const = 0;
    virtual constexpr std::string_view msg() const = 0;
    virtual restinio::http_status_line_t status() const = 0;
    [[nodiscard]] nlohmann::json json() const {
        nlohmann::json j;
        j["error_id"] = this->id();
        j["message"] = this->msg();

        if (!this->info.is_null() || !this->info.empty())
            j["info"] = this->info;
        return j;
    }
protected:
    ~Error() override = default;
};

/* Compile type concept (trait) for what is Error */
template<typename E>
concept CError = std::same_as<E, Error> || std::derived_from<E, Error>;

struct OtherError final : Error {
    using Error::Error;
    [[nodiscard]] constexpr std::string_view id() const override { return "OtherError"; }
    [[nodiscard]] constexpr std::string_view msg() const override { return "Other error"; }
    [[nodiscard]] inline restinio::http_status_line_t status() const override { return restinio::status_internal_server_error(); }
};

struct InvalidParamsError final : Error {
    using Error::Error;
    [[nodiscard]] constexpr std::string_view id() const override { return "InvalidParamsError"; }
    [[nodiscard]] constexpr std::string_view msg() const override { return "Invalid Parameters"; }
    [[nodiscard]] inline restinio::http_status_line_t status() const override { return restinio::status_bad_request(); }
};

struct JsonParseError final : Error {
    using Error::Error;
    [[nodiscard]] constexpr std::string_view id() const override { return "JsonParseError"; }
    [[nodiscard]] constexpr std::string_view msg() const override { return "Error parsing JSON"; }
    [[nodiscard]] inline restinio::http_status_line_t status() const override { return restinio::status_bad_request(); }
};

struct NotFoundError final : Error {
    using Error::Error;
    [[nodiscard]] constexpr std::string_view id() const override { return "NotFoundError"; }
    [[nodiscard]] constexpr std::string_view msg() const override { return "Reource not found"; }
    [[nodiscard]] inline restinio::http_status_line_t status() const override { return restinio::status_not_found(); }
};

struct DBError final : Error {
    using Error::Error;
    [[nodiscard]] constexpr std::string_view id() const override { return "DBError"; }
    [[nodiscard]] constexpr std::string_view msg() const override { return "Database error"; }
    [[nodiscard]] inline restinio::http_status_line_t status() const override { return restinio::status_bad_request(); }
};

struct UnauthorizedError final : Error {
    using Error::Error;
    [[nodiscard]] constexpr std::string_view id() const override { return "UnauthorizedError"; }
    [[nodiscard]] constexpr std::string_view msg() const override { return "Invalid permissions"; }
    [[nodiscard]] inline restinio::http_status_line_t status() const override { return restinio::status_forbidden(); }
};

struct InvalidAuthTokenError final : Error {
    using Error::Error;
    [[nodiscard]] constexpr std::string_view id() const override { return "InvalidAuthToken"; }
    [[nodiscard]] constexpr std::string_view msg() const override { return "Invalid authentication token"; }
    [[nodiscard]] inline restinio::http_status_line_t status() const override { return restinio::status_bad_request(); }
};

struct InvalidRefreshTokenError final : Error {
    using Error::Error;
    [[nodiscard]] constexpr std::string_view id() const override { return "InvalidRefreshToken"; }
    [[nodiscard]] constexpr std::string_view msg() const override { return "Invalid refresh token"; }
    [[nodiscard]] inline restinio::http_status_line_t status() const override { return restinio::status_bad_request(); }
};

} // ns rs

template<rs::CError E>
struct nlohmann::adl_serializer<E> {
    static void to_json(nlohmann::json &j, const E& e) {
        j = e.json();
   }
};

#endif 
