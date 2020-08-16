#ifndef USER_HPP
#define USER_HPP

#include <string>
#include <functional>
#include <ctype.h>
#include <nlohmann/json.hpp>
#include <jwt/jwt.hpp>

#include "errors.hpp"
#include "utils.hpp"

#include "3rd_party/magic_enum.hpp"

#include "model/model.hpp"
#include "models.hpp"

namespace rs {
enum class UserGroup {
    other = 0,
    owner = 1,
    guest = 2,
    user = 3,
    admin = 4
};
constexpr unsigned num_of_user_groups = magic_enum::enum_count<UserGroup>();
constexpr const char * group_name(UserGroup g) {
    return magic_enum::enum_name(g).data();
}

constexpr bool have_exact_permissions(uint8_t desired, uint8_t perms) {
    return (desired ^ perms) == 0;
}

constexpr bool have_permissions(uint8_t desired, uint8_t perms) {
    return (desired & perms) == desired;
}

namespace permission {
    constexpr uint8_t CREATE = 0b1000;
    constexpr uint8_t READ = 0b0100;
    constexpr uint8_t UPDATE = 0b0010;
    constexpr uint8_t DELETE = 0b0001;
};

struct PermissionParams {
    rs::UserGroup group_id = UserGroup::guest;
    std::optional<uint64_t> user_id;
    std::optional<std::string> owner_field_name;

    PermissionParams without_owner() const {
        PermissionParams result = *this;
        result.user_id = {};
        result.owner_field_name = {};
        return result;
    }

    PermissionParams without_group() const {
        PermissionParams result = *this;
        result.group_id = UserGroup::other;
        return result;
    }
};

std::string permissions_to_string(uint8_t permissions) {
    std::string str;
    using namespace permission;
    str.append(1, have_permissions(CREATE, permissions) ? 'C' : '-');
    str.append(1, have_permissions(READ, permissions) ? 'R' : '-');
    str.append(1, have_permissions(UPDATE, permissions) ? 'U' : '-');
    str.append(1, have_permissions(DELETE, permissions) ? 'D' : '-');
    return str;
}

nlohmann::json permissions_to_json(uint8_t perms) {
    nlohmann::json j;
    j["required_permissions"] = permissions_to_string(perms);
    return j;
};

template <typename M>
auto permission_matrix_json(auto ps) {
    nlohmann::json result_json;
    std::array<std::string, ps[0].size()> str_ps;

    for (auto i=0; i < ps.size(); i++) {
        std::transform(std::begin(ps[i]), std::end(ps[i]), std::begin(str_ps), permissions_to_string);
        auto gname = group_name(static_cast<UserGroup>(i));
        result_json[gname]["instance"] = std::move(str_ps[0]);
        for (auto j=0u; j < M::num_of_fields(); j++) {
            result_json[gname][M::field_name(j)] = std::move(str_ps[j+1]);
        }
    }
    return result_json;
}

template <model::CModel M>
class AuthorizedModelAccess {
    using permissions_matrix_t = std::array<std::array<uint8_t, M::num_of_fields()+1>,rs::num_of_user_groups>;
    M &&m_model;
    uint8_t m_desired_permissions;
    PermissionParams m_permission_params;
    permissions_matrix_t m_permissions_matrix;

    void grant_permission_params_from_auth_token(soci::session &db, const model::AuthToken &auth_token, PermissionParams &pp) {
        if (!auth_token.auth_token.opt_value.has_value())
            return;

        const auto auth_tok = *auth_token.auth_token.opt_value;
        const auto decoded = jwt::decode(auth_tok, jwt::params::algorithms({"HS256"}), jwt::params::secret("changemesecret"));
        throw_if<InvalidAuthTokenError>(!decoded.has_claim("group_id") && !decoded.has_claim("user_id"), "Token does not have required claims");
        const auto payload_user_id = decoded.payload().get_claim_value<int32_t>("user_id");
        std::string db_auth_token;
        db << fmt::format("SELECT auth_token FROM auth_tokens WHERE user_id = '{}'", payload_user_id), soci::into(db_auth_token);
        throw_if<InvalidAuthTokenError>(auth_tok != db_auth_token);
        const auto payload_group_id = decoded.payload().get_claim_value<int32_t>("group_id");
        pp.group_id = static_cast<UserGroup>(payload_group_id);
        pp.user_id = payload_user_id;
    }

    void check_instance_permissions() {
        uint8_t group_instance_perms = m_permissions_matrix[static_cast<uint8_t>(m_permission_params.group_id)][0];
        uint8_t owner_instance_perms = m_permissions_matrix[static_cast<uint8_t>(UserGroup::owner)][0];

        bool have_group_perms = have_permissions(m_desired_permissions, group_instance_perms);
        bool have_owner_perms = m_permission_params.user_id.has_value() && m_permission_params.owner_field_name.has_value()
                              && have_permissions(m_desired_permissions, owner_instance_perms);

        throw_if<UnauthorizedError>(!have_group_perms && !have_owner_perms, permissions_to_json(m_desired_permissions));

        if (!have_group_perms) {
            m_permission_params.group_id = UserGroup::other; 
        }

        if (!have_owner_perms) {
            m_permission_params.owner_field_name = {};
            m_permission_params.user_id = {};
        }
    }

    void load_perms_from_db(soci::session &db, std::string_view table_name) {
        soci::rowset<soci::row> rows = (db.prepare << fmt::format("SELECT * FROM {}", std::string{table_name} + "_permissions"));
        std::for_each(std::begin(rows), std::end(rows),
            [&](const auto &row) {
                auto row_id = row.template get<int>("group_id");
                m_permissions_matrix[row_id][0] = row.template get<int>("instance");
                for (auto i = 0u; i < M::num_of_fields(); i++) {
                    try {
                        m_permissions_matrix[row_id][i+1] = row.template get<int>(M::field_name(i));
                    } catch (const soci::soci_error &e) {
                        // Ignoring error if field does not exist
                    }
                }
        });
    }

    void erase_unauthorized_fields() {
        auto perms = m_permissions_matrix[static_cast<unsigned>(m_permission_params.group_id)];
        if (m_permission_params.owner_field_name.has_value() && m_permission_params.user_id.has_value()) {
            auto resource_owner_id = m_model.template field_opt_value<std::optional<int>>(M::field_index(m_permission_params.owner_field_name->c_str()));
            if (resource_owner_id == *m_permission_params.user_id) {
                std::transform(std::cbegin(perms), std::cend(perms), 
                               std::cbegin(m_permissions_matrix[static_cast<uint8_t>(UserGroup::owner)]),
                               std::begin(perms), std::bit_or{});
            }
        }

        int num_of_erased_fields = 0;
        for (auto i=1u; i < perms.size(); i++) {
            if (!have_permissions(m_desired_permissions, perms[i])) {
                 m_model.template erase_value(i-1);
                 num_of_erased_fields++;
            }
        }
        rs::throw_if<UnauthorizedError>(num_of_erased_fields == M::num_of_fields(), permissions_to_json(m_desired_permissions));
    }
    public:
    AuthorizedModelAccess(uint8_t desired_permissions, model::AuthToken auth_tok, PermissionParams pp, soci::session &db, std::string_view table_name, M &&m = {}) 
        : m_model(std::move(m)),
          m_desired_permissions(desired_permissions),
          m_permission_params(std::move(pp)) {
        grant_permission_params_from_auth_token(db, auth_tok, m_permission_params);
        load_perms_from_db(db, table_name);
        check_instance_permissions();
    }

    M&& get_safely() {
        erase_unauthorized_fields();
        return std::move(m_model);
    }

    M& unsafe_ref() {
        return m_model;
    }
};
} // ns rs

#endif // USER_HPP
