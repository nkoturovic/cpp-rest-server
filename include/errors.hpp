#ifndef RS_ERRORS_HPP
#define RS_ERRORS_HPP

#include "typedefs.hpp"

namespace rs {

class Exception : std::exception {
protected:
    Exception(json_t &&info) : info(std::move(info)) {}
public:
    json_t info;

    virtual ~Exception() = default;
    constexpr virtual const char * id() const = 0;
    constexpr virtual const char * msg() const = 0;
    constexpr virtual restinio::http_status_line_t status() const = 0;
    const char * what() const noexcept override {
        return this->msg();
    }

    json_t json() const {
        json_t j;
        j["error_id"] = this->id();
        j["message"] = this->msg();

        if (!info.is_null() || !info.empty())
            j["info"] = info;

        return j;
    }
};

template<typename C>
concept CException = std::derived_from<C,Exception>;

class Error final : public Exception {
public:
    Error(json_t &&info = {}) : Exception(std::move(info)) {}
    constexpr const char * id() const override { return "OtherError"; }
    constexpr const char * msg() const override { return "Other error"; }
    restinio::http_status_line_t status() const override { return restinio::status_internal_server_error(); }
};

class InvalidParamsError final : public Exception {
public:
    InvalidParamsError(json_t &&info = {}) : Exception(std::move(info)) {}
    constexpr const char * id() const override { return "InvalidParams"; }
    constexpr const char * msg() const override { return "Invalid Parameters"; }
    restinio::http_status_line_t status() const override { return restinio::status_bad_request(); }
};

class JsonParseError final : public Exception {
public:
    JsonParseError(json_t &&info = {}) : Exception(std::move(info)) {}
    constexpr const char * id() const override { return "JsonParseError"; }
    constexpr const char * msg() const override { return "Error parsing JSON"; }
    restinio::http_status_line_t status() const override { return restinio::status_bad_request(); }
};

class NotFoundError final : public Exception {
public:
    NotFoundError(json_t &&info = {}) : Exception(std::move(info)) {}
    constexpr const char * id() const override { return "NotFound"; }
    constexpr const char * msg() const override { return "Resource not found"; }
    restinio::http_status_line_t status() const override { return restinio::status_not_found(); }
};

class DBError final : public Exception {
public:
    DBError(json_t &&info = {}) : Exception(std::move(info)) {}
    constexpr const char * id() const override { return "DBError"; }
    constexpr const char * msg() const override { return "Database Error"; }
    restinio::http_status_line_t status() const override { return restinio::status_internal_server_error(); }
};

} // ns rs

#endif 
