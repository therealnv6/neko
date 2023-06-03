#include <interpreter.hpp>
#include <token.hpp>

int main()
{
	std::string program = R"(
    let x: int = 5
    let y: int = 93193921
    let z: int = 7
  )";

	interpreter interpreter { program };
	interpreter.run();

	std::cout << "x: " << interpreter.get_value_at("x").second << std::endl;
	std::cout << "y: " << interpreter.get_value_at("y").second << std::endl;
	std::cout << "z: " << interpreter.get_value_at("z").second << std::endl;
}
