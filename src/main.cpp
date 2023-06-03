#include <fstream>
#include <interpreter.hpp>
#include <iostream>
#include <token.hpp>

int main()
{
	std::ifstream inputFile("./src/example.neko");
	std::string program((std::istreambuf_iterator<char>(inputFile)),
		std::istreambuf_iterator<char>());

	interpreter interpreter { program };
	interpreter.run();

	for (const auto &entry : interpreter.get_values())
	{
		const auto &key = entry.first;
		const auto &[type, value] = entry.second;

		std::cout << key << ": (" << type << ") " << value << std::endl;
	}

	return 0;
}
