#pragma once
#include "token.hpp"
#include <lexer.hpp>
#include <stdexcept>

class interpreter
{
	typedef void (interpreter::*callback)();

private:
	std::string program;
	lex::lexer lexer;

	token current_token = {};

	std::map<std::string, std::pair<std::string, std::string>> variables;

	std::map<std::string, std::pair<int, int>> function_addresses;
	std::map<std::string, std::map<std::string, token_type>> function_token_map;

	std::map<token_type, callback> func_map;

public:
	interpreter(const std::string &program)
		: program(program)
		, lexer(program)
	{
		func_map.emplace(token_type::let, &interpreter::handle_let);
		func_map.emplace(token_type::print, &interpreter::handle_print);
		func_map.emplace(token_type::function, &interpreter::handle_function);
	}

	void run()
	{
		current_token = lexer.get_next_token();

		while (current_token.type != token_type::eof)
		{
			auto function = func_map.find(current_token.type);

			if (function != func_map.end())
			{
				auto function = func_map[current_token.type];
				(this->*function)();
			}

			current_token = lexer.get_next_token();
		}
	}

	std::pair<std::string, std::string> get_value_at(const std::string &key)
	{
		return variables[key];
	}

	std::map<std::string, std::pair<std::string, std::string>> get_values()
	{
		return variables;
	}

private:
	void handle_let()
	{
		lex::let_data data = lexer.lex_let();

		variables[data.var.name] = std::make_pair(data.var.type_container.type_name, data.value->data());
	}

	void handle_function()
	{
		lex::function_data data = lexer.lex_function();

		std::cout << "function address for "
				  << data.function_name
				  << ": "
				  << data.address.first
				  << std::endl;
	}

	void handle_print()
	{
		token value_token = lexer.get_next_token();
		expect_token(token_type::string, value_token);

		std::string output = value_token.value;
		std::size_t pos = 0;

		while ((pos = output.find("${", pos)) != std::string::npos)
		{
			std::size_t end_pos = output.find("}", pos + 2);
			std::string variable_name = output.substr(pos + 2, end_pos - pos - 2);

			if (variables.count(variable_name) > 0)
			{
				std::string variable_value = variables[variable_name].second;
				output.replace(pos, end_pos - pos + 1, variable_value);
				pos += variable_value.length();
			}
			else
			{
				pos += end_pos - pos + 1;
			}
		}

		std::cout << output << std::endl;
	}

	void expect_token(token_type expected_type, const token &token)
	{
		if (token.type != expected_type)
		{
			throw std::runtime_error(
				"Syntax error: Unexpected token: " + token.value
				+ ". Expected token type: "
				+ std::to_string(static_cast<int>(expected_type)));
		}
	}
};
