#pragma once
#include "token.hpp"
#include <fstream>
#include <iostream>
#include <lexer.hpp>
#include <stdexcept>

class compiler
{
	typedef void (compiler::*callback)(token &);

private:
	std::string program;
	lex::lexer lexer;
	std::ofstream output_file;

	std::map<std::string, std::pair<std::string, std::string>> variables;

	std::map<token_type, callback> func_map {
		{		  token_type::let,		   &compiler::handle_let},
		{		  token_type::print,		 &compiler::handle_print},
		{	  token_type::function,		&compiler::handle_function},
		{token_type::function_call, &compiler::handle_function_call}
	};

public:
	compiler(const std::string &program, const std::string &outputFilename)
		: program(program)
		, lexer(program)
		, output_file(outputFilename)
	{
	}

	void compile()
	{
		output_file << ".section .data" << std::endl;

		token current_token = lexer.get_next_token();
		while (current_token.type != token_type::eof)
		{
			if (current_token.type == token_type::let)
			{
				lex::let_data data = lexer.lex_let();
				variables[data.var.name] = std::make_pair(
					data.var.type_container.type_name,
					data.value->data());
			}

			current_token = lexer.get_next_token();
		}

		output_file << ".section .text" << std::endl;
		output_file << ".global _start" << std::endl;
		output_file << "_start:" << std::endl;

		lexer.set_token_pos(0);
		current_token = lexer.get_next_token();

		while (current_token.type != token_type::eof)
		{
			if (current_token.type == token_type::print)
			{
				handle_print(current_token);
			}

			current_token = lexer.get_next_token();
		}

		output_file.close();
	}

private:
	void handle_print(token &current)
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

		output_file << "mov rax, 1" << std::endl;
		output_file << "mov rdi, 1" << std::endl;
		output_file << "mov rsi, output_string" << std::endl;
		output_file << "mov rdx, " << output.length() << std::endl;
		output_file << "syscall" << std::endl;

		output_file << ".section .data" << std::endl;
		output_file << "output_string: db '" << output << "', 0" << std::endl;
	}

	void handle_function_call(token &)
	{
	}

	void handle_function(token &)
	{
	}

	void handle_let(token &)
	{
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
