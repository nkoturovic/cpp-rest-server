#ifndef RS_ERRORS_HPP
#define RS_ERRORS_HPP

#include "typedefs.hpp"

namespace rs {
enum class ApiErrorId { 
    Other,
    InvalidParams,
    JsonParseError,
    NotFound,
    DBError 
};

constexpr inline const char * msgFromApiErrorId(ApiErrorId id) {
    switch(id) {
        case ApiErrorId::InvalidParams : return "Invalid parameters";
        case ApiErrorId::JsonParseError : return "Error while parsing JSON request";
        case ApiErrorId::DBError : return "Error with db query";
        case ApiErrorId::NotFound : return "Api Request not Found";
        default : return "Other error";
    }
}

inline restinio::http_status_line_t statusCodeFromApiErrorId(ApiErrorId id) {
    switch(id) {
        case ApiErrorId::InvalidParams : return restinio::status_bad_request();
        case ApiErrorId::JsonParseError : return restinio::status_bad_request();
        case ApiErrorId::DBError : return restinio::status_internal_server_error();
        case ApiErrorId::NotFound : return restinio::status_not_found();
        default : return restinio::status_internal_server_error();
    }
}

class ApiError {
    ApiErrorId m_id;
    json_t m_info;
public:
    ApiErrorId id() const { return m_id; }
    ApiError(ApiErrorId id, json_t && info = nullptr) : m_id(id), m_info(std::move(info)) {}
    json_t json() const {
        json_t j;
        j["error_id"] = magic_enum::enum_name(m_id);
        j["message"] = msgFromApiErrorId(m_id);

        if (!m_info.is_null())
            j["info"] = m_info;

        return j;
    }
    const char * msg() const {
        return msgFromApiErrorId(m_id); 
    }

    restinio::http_status_line_t status() const {
        return statusCodeFromApiErrorId(m_id); 
    }
};

class ApiException : public ApiError, public std::exception {
 public:
    ApiException(ApiErrorId id, json_t && info = { nullptr }) : ApiError(id, std::move(info)) {}
    const char * what() const noexcept override { return this->msg(); }
};

}

#endif 
