#ifndef RS_UTILS_HPP
#define RS_UTILS_HPP

#include <fstream>
#include <string_view>
#include <filesystem>
#include <span>
#include <restinio/router/easy_parser_router.hpp>
#include <restinio/helpers/file_upload.hpp>
#include <restinio/helpers/multipart_body.hpp>
#include <boost/lexical_cast.hpp>
#include <random>
#include <iomanip>
#include "errors.hpp"

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
    std::optional<const char*> address;
    std::optional<unsigned> port;
};

CmdLineArgs parse_cmdline_args(std::span<char *> args)
{
    CmdLineArgs result{};
    auto it_end = std::cend(args);

    for (auto it = std::cbegin(args); it != it_end; it++) {
        auto it_next = std::next(it);
        std::string_view curr{*it};
        if ((curr == "--address" || curr == "-i") && it_next != it_end)
            result.address = *it_next;
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

nlohmann::json extract_json_field(const restinio::request_handle_t & req) {
    using namespace restinio::multipart_body;
    const auto boundary = detect_boundary_for_multipart_body(*req, "multipart", "form-data" );
    if (boundary) {
       const auto parts = split_multipart_body(req->body(), *boundary );
       for (restinio::string_view_t part : parts) {
          const auto parsed_part = try_parse_part(part);
          throw_if<InvalidParamsError>(!parsed_part.has_value(), "json field is empty");
          throw_if<InvalidParamsError>(!parsed_part->fields.has_field("json"), "json field is empty");
          return nlohmann::json{parsed_part->fields.get_field("json")};
       }
    }
    throw_if<InvalidParamsError>(true, "json field not found");
    return nullptr;
}

std::string iso_date_now() {
    std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss << std::put_time( std::localtime( &t ), "%F" );
    return ss.str();
}

std::string iso_date_time_now() {
    std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss << std::put_time( std::localtime( &t ), "%FT%T%z" );
    return ss.str();
}

// Taken from https://github.com/Stiffstream/restinio/blob/master/dev/sample/file_upload/main.cpp
// and modifed for purpose of this application
static void store_file_to_disk(
	std::string_view dest_folder,
	std::string_view file_name,
	std::string_view raw_content)
{
	std::ofstream dest_file;
	dest_file.exceptions( std::ofstream::failbit );
	dest_file.open(
			fmt::format( "{}/{}", dest_folder, file_name ),
			std::ios_base::out | std::ios_base::trunc | std::ios_base::binary );
	dest_file.write( raw_content.data(), raw_content.size() );
}

struct MFile {
    std::string file_name;
    std::string file_extension;
    std::string file_contents;
};

MFile parse_file_field_multiform(const restinio::request_handle_t & req)
{
	using namespace restinio::file_upload;
    std::optional<MFile> result_file;
	enumerate_parts_with_files(
        *req, [&](const part_description_t &part) {
            if(part.name == "file" && part.filename.has_value() 
                    && !part.body.empty()) {
                result_file = {
                    .file_name = *part.filename,
                    .file_contents = std::string{part.body}
                };
            }
            return handling_result_t::terminate_enumeration;
        });

    throw_if<InvalidParamsError>(!result_file.has_value(), "File is required");
    result_file->file_extension = std::filesystem::path(result_file->file_name).extension();
    return *result_file;
}

auto parse_json_field_multiform(const restinio::request_handle_t & req)
{
	using namespace restinio::multipart_body;
    std::optional<nlohmann::json> json;
    auto hstatus = handling_result_t::continue_enumeration;
	auto parts = enumerate_parts(
        *req, [&](const parsed_part_t &part) {
            part.fields.for_each_field([&](const restinio::http_header_field_t& f) {
                if (hstatus == handling_result_t::stop_enumeration) {
                    return;
                } else if (auto pos = f.value().find("name=\""); pos != std::string::npos) {
                    if ((f.value().length() - pos) >= 10) {
                        std::string substr = f.value().substr(pos+6, 4);
                        if (substr == "json") {
                            json = nlohmann::json::parse(part.body);
                            hstatus = handling_result_t::stop_enumeration;
                        }
                    }
                }
            });
        return hstatus;
    });

    throw_if<InvalidParamsError>(!json.has_value(), "json field is required");
    return *json;
}

int randint() {
    std::random_device dev;
    std::mt19937 rgen(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0,INT_MAX);
    return dist(rgen); 
}
} // ns rs

#endif
