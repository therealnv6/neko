#include <interpreter.hpp>
#include <token.hpp>

int main()
{
	std::string program = R"(
      let mommy: int = 9
      let faggot: str = "hi faggotsss"
    )";

	interpreter interpreter { program };
	interpreter.run();

	for (auto entry : interpreter.get_values())
	{
		auto key = entry.first;
		auto [type, value] = entry.second;

		std::cout << key << ": (" << type << ") " << value << std::endl;
	}
}
