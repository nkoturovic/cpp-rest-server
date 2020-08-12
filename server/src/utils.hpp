#ifndef RS_UTILS_HPP
#define RS_UTILS_HPP

#include <string_view>
#include <span>
#include <restinio/router/easy_parser_router.hpp>

#include "errors.hpp"
#include <boost/lexical_cast.hpp>

/* This code is fixing boost::lexical_cast true -> 1 and false -> 0 */
namespace boost {
    template<> 
    bool lexical_cast<bool, std::string>(const std::string& arg) {
        std::istringstream ss(arg);
        bool b;
        ss >> std::boolalpha >> b;
        return b;
    }

    template<>
    std::string lexical_cast<std::string, bool>(const bool& b) {
        std::ostringstream ss;
        ss << std::boolalpha << b;
        return ss.str();
    }
}

namespace rs {
template <typename T> constexpr std::string_view type_name;
template <> constexpr const char * type_name<int> = "int";
template <> constexpr const char * type_name<char> = "char";
template <> constexpr const char * type_name<long> = "long int";
template <> constexpr const char * type_name<unsigned> = "unsigned int";
template <> constexpr const char * type_name<unsigned long> = "unsigned long int";
template <> constexpr const char * type_name<float> = "float";
template <> constexpr const char * type_name<double> = "double";
template <> constexpr const char * type_name<bool> = "bool";
template <> constexpr const char * type_name<std::string> = "string";
template <> constexpr const char * type_name<std::string_view> = "string";
template <> constexpr const char * type_name<const char *> = "string";

nlohmann::json success_response(std::string_view info = "") {
    nlohmann::json json;
    json["message"] = "SUCCESS";
    if (!info.empty())
        json["info"] = info;
    return json;
};

template <CError E>
constexpr void throw_if(bool condition, nlohmann::json &&info = {}) {
    if (condition)
        throw E(std::move(info));
}

struct CmdLineArgs {
    const char * ip = "localhost";
    unsigned port = 3000;
};

CmdLineArgs parse_cmdline_args(std::span<char *> args) 
{
    CmdLineArgs result{};
    auto it_end = std::cend(args);

    for (auto it = std::cbegin(args); it != it_end; it++) {
        auto it_next = std::next(it);
        std::string_view curr{*it};
        if ((curr == "--ip" || curr == "-i") && it_next != it_end)
            result.ip = *it_next;
        else if ((curr == "--port" || curr == "-p") && it_next != it_end)
            result.port = std::strtoul(*it_next, nullptr, 10);
    }
    return result;
};

template <typename T>
struct function_traits
    : public function_traits<decltype(&T::operator())>
{};
// For generic types, directly use the result of the signature of its 'operator()'

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) const>
// we specialize for pointers to member function
{
    enum { arity = sizeof...(Args) };
    // arity is the number of arguments.

    using result_type = ReturnType;

    template <size_t i>
    struct arg
    {
        using type = typename std::tuple_element<i, std::tuple<Args...>>::type;
        // the i-th argument is equivalent to the i-th tuple element of a tuple
        // composed of those arguments.
    };
};

void register_api_reference_route(auto &router, std::string_view path) {
    router.epr->http_get(restinio::router::easy_parser_router::path_to_params(path), 
        [&](const auto &req) {
            std::string content = {"<!DOCTYPE html><html></body><h1>API reference</h1>"};
            for (const auto &r : router.registered_routes_info) {
                content.append(fmt::format("<b>{}</b>:&nbsp;&nbsp;{}<br><code>", r.method_id, r.url));
                for (const auto &[k,v] : r.params_description) {
                    content.append(fmt::format("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{} : {} &lt;", k, v.type));
                    if (v.cnstr_names.begin() != v.cnstr_names.end()) {
                        content.append(std::accumulate(v.cnstr_names.begin() + 1, 
                                        v.cnstr_names.end(), 
                                        std::string{*v.cnstr_names.begin()}, 
                                        [](std::string acc, std::string_view s) {
                                           return acc + ", " + std::string(s); 
                                        }
                        ));
                    }
                    content.append("&gt;<br>");
                }
                content.append("</code><br>");
            }
            content.append("</body></html>");
            return req->create_response(restinio::status_ok())
               .append_header(restinio::http_field::content_type, "text/html")
               .set_body(content)
               .done();
    });
}
} // ns rs

#endif 
