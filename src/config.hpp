#include <fstream>
#include "typedefs.hpp"

namespace rs {
class ServerConfig {
    std::string m_address = "localhost";
    unsigned m_port = 80;
public:
    ServerConfig(nlohmann::json json) {
        try {
            json["address"].get_to(m_address);
            json["port"].get_to(m_port);
        } catch (const std::exception &e) {
            std::clog << __FILE__ << ": " 
                      << __FUNCTION__ << '\n';
            std::clog << e.what() << '\n';
        }
    }

    ServerConfig(const char *path) {
         if (std::ifstream in_json_file(path); in_json_file.is_open()) {
             nlohmann::json json;
            in_json_file >> json;
            *this = ServerConfig(json);
         }
    }

    const std::string& ip() { return m_address; }
    unsigned port() { return m_port; }
};
}
