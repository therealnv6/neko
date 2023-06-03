#pragma once
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <token.hpp>
#include <type_traits>
#include <unordered_map>

class interpreter
{
	typedef void (interpreter::*function)();

private:
	std::string program;
	std::size_t pos = 0;

	token current_token = {};

	std::map<std::string, std::pair<std::string, std::string>> variables;
	std::map<token_type, function> func_map;
	std::unordered_map<std::string, token_type> char_tokens = {
		{":",	   token_type::colon},
		{"=",	   token_type::eq},
		{"+",	   token_type::add},
		{"-",	   token_type::minus},
		{"*", token_type::asterisk},
		{"/",	   token_type::slash},
	};

public:
	interpreter(const std::string &program)
		: program(program)
	{
		func_map.emplace(token_type::let, &interpreter::handle_let_statement);
		func_map.emplace(token_type::print, &interpreter::handle_print_statement);
	}

	void run()
	{
		current_token = get_next_token();

		while (current_token.type != token_type::eof)
		{
			auto function = func_map.find(current_token.type);

			if (function != func_map.end())
			{
				// find a better way to do this
				auto function = func_map[current_token.type];
				(this->*function)();
			}

			current_token = get_next_token();
		}
	}

	std::pair<std::string, std::string> get_value_at(std::string key)
	{
		return variables[key];
	}

private:
	token get_next_token()
	{
		if (pos >= program.length())
		{
			return { token_type::eof, "" };
		}

		char current_char = program[pos];
		char next_char = (pos + 1 < program.length()) ? program[pos + 1] : '\0';

		if (std::isdigit(current_char))
		{
			std::string number;
			number += current_char;

			while (next_char != '\0' && std::isdigit(next_char))
			{
				pos++;
				number += next_char;
				next_char = (pos + 1 < program.length()) ? program[pos + 1] : '\0';
			}

			pos++;
			return { token_type::integer, number };
		}

		if (std::isalpha(current_char))
		{
			std::string identifier;
			identifier += current_char;

			while (next_char != '\0' && (std::isalpha(next_char) || std::isdigit(next_char)))
			{
				pos++;
				identifier += next_char;
				next_char = (pos + 1 < program.length()) ? program[pos + 1] : '\0';
			}

			pos++;

			if (identifier == "let")
			{
				return { token_type::let, identifier };
			}
			else if (identifier == "print")
			{
				return { token_type::print, identifier };
			}
			else
			{
				return { token_type::ident, identifier };
			}
		}

		pos++;

		auto token = char_tokens.find(std::to_string(current_char));

		if (token != char_tokens.end())
		{
			return {
				token->second, token->first
			};
		}

		switch (current_char)
		{
		case '"':
			{
				std::string str;
				while (next_char != '\0' && next_char != '"')
				{
					pos++;
					str += next_char;
					next_char = (pos + 1 < program.length()) ? program[pos + 1] : '\0';
				}
				pos++; // Consume the closing quotation mark
				return { token_type::str, str };
			}

		case 0x20:
		case 0x0c:
		case 0x0a:
			return get_next_token();
		default:
			return { token_type::eof, "" };
		}
	}

	void handle_let_statement()
	{
		token identifier_token = get_next_token(); // Expecting an identifier
		expect_token(token_type::ident, identifier_token);
		expect_token(token_type::colon, get_next_token());

		token type_token = get_next_token(); // Expecting a type
		expect_token(token_type::ident, type_token);
		expect_token(token_type::eq, get_next_token());

		token value_token = get_next_token(); // Expecting a value
		expect_token(token_type::integer, value_token);

		variables[identifier_token.value] = std::make_pair(type_token.value, value_token.value);
	}

	void handle_print_statement()
	{
		token value_token = get_next_token(); // Expecting a value
		expect_token(token_type::str, value_token);

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
		if (token.type == token_type::eof)
		{
			return;
		}

		if (token.type != expected_type)
		{
			throw std::runtime_error(
				"Syntax error: Unexpected token: " + token.value
				+ ". Expected token type: "
				+ std::to_string(static_cast<int>(expected_type)));
		}
	}
};
