#pragma once
#include "token.hpp"
#include <lexer.hpp>
#include <stdexcept>

class interpreter
{
	typedef void (interpreter::*callback)(token &);

private:
	std::string program;
	lex::lexer lexer;

	token current_token = {};

	int stack_limit_switch = -1;
	int previous_stack = -1;

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
		func_map.emplace(token_type::function_call, &interpreter::handle_function_call);
	}

	void run()
	{
		if (lexer.get_token_pos() >= stack_limit_switch && stack_limit_switch != -1)
		{
			lexer.set_token_pos(previous_stack);
			stack_limit_switch = -1;
		}

		current_token = lexer.get_next_token();

		while (current_token.type != token_type::eof)
		{
			// there should be a better way to do this, thoughts?
			if (lexer.get_token_pos() >= stack_limit_switch && stack_limit_switch != -1)
			{
				lexer.set_token_pos(previous_stack);
				stack_limit_switch = -1;
			}

			// we have this variable to find it first, so we can check if it exists.
			auto function = func_map.find(current_token.type);

			if (function != func_map.end())
			{
				// we might be able to call the function variable above, but i couldn't find a way.
				auto function = func_map[current_token.type];
				(this->*function)(current_token);
			}

			// we set current_token to the next token, this is for the next iteration.
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
	void handle_let(token &)
	{
		lex::let_data data = lexer.lex_let();

		variables[data.var.name] = std::make_pair(
			data.var.type_container.type_name,
			data.value->data() // this is optional, we might want to make a check in the future. for now, the optionality is not implemented.
		);
	}

	void handle_function(token &)
	{
		lex::function_data data = lexer.lex_function();

		function_addresses.emplace(data.function_name, data.address);

		std::map<std::string, token_type> tokens;

		for (auto token : data.token_data)
		{
			auto token_type = token.type_container.type;
			auto variable_name = token.name;

			tokens.emplace(variable_name, token_type);
		}

		function_token_map.emplace(data.function_name, tokens);
	}

	void handle_function_call(token &current)
	{
		auto call_data = lexer.lex_function_call(current, function_token_map);
		previous_stack = lexer.get_token_pos();

		auto call_site = function_addresses.find(current.value);

		if (call_site == function_addresses.end())
		{
			throw std::runtime_error("unknown function: " + current.value);
		}

		auto [begin, end] = call_site->second;

		for (auto token : call_data.token_data)
		{
			variables[token.var.name] = std::make_pair(
				token.var.type_container.type_name,
				token.value->data() // could cause a segfault because it's an optional<>. todo: implement actual optional values
			);
		}

		lexer.set_token_pos(begin);
		stack_limit_switch = end;
	}

	void handle_print(token &)
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
