#include <iostream>
#include <string>
#include <regex>
#include <nlohmann/json.hpp>

#include "model/constraint.hpp"
#include "model/field.hpp"
#include "model/model.hpp"

struct ISODate {
    using value_type = std::string;
    ISODate() = delete;

    static bool is_satisfied(const std::string& s) {
        const std::regex pattern
                (R"(([12]\d{3}-(0[1-9]|1[0-2])-(0[1-9]|[12]\d|3[01])))");
        return std::regex_match(s, pattern);
    }
    constexpr static const char * name = "ISODate";
    constexpr static const char * description = "Date format is yyyy-mm-dd";
};

using namespace rs::model;

struct ForumPost : rs::model::Model<ForumPost> {
	Field<std::string, cnstr::Length<2,20>> post_name;
	Field<std::string, cnstr::Required, ISODate> added_date;
	Field<std::string> content;
};

/* Required for REFLECTION!! */
/* Models for Database */
REFL_AUTO(
  type(ForumPost),
  field(post_name),
  field(added_date),
  field(content)
)

int main()
{
	ForumPost post{
		.post_name = {"Hello world"},
		.added_date = {"2020-08-10"},
		.content = {R"(printf("Hello world\n"))"}
	};
	
	std::cout << nlohmann::json{post.get_description()}.dump(2) << std::endl;
	std::cout << post.json().dump(2) << std::endl;

	return 0;	  
}
