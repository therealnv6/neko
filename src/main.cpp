#include <fstream>

#include <compiler.hpp>
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

	compiler compiler(program, "build/neko-built");
	compiler.compile();
}
