#pragma once
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <token.hpp>
#include <unordered_map>
#include <vector>

namespace lex
{
	struct type_data {
		token_type type;
		std::string type_name;
	};

	struct var_data {
		type_data type_container;
		std::string name;
	};

	struct function_data {
		std::string function_name;
		std::pair<int, int> address;
		std::vector<var_data> token_data;
		std::map<std::string, int> token_data_index;
	};

	struct let_data {
		var_data var;
		std::optional<std::string> value;
	};

	struct function_call_data {
		std::string function_name;
		std::vector<let_data> token_data;
	};

	struct expression {
		expr_type type;
		std::string value;
		std::shared_ptr<expression> left;
		std::shared_ptr<expression> right;
	};

	enum lexing_state
	{
		function_sig,
		function_body,
		function_call,
		unknown,
	};

	class lexer
	{
	private:
		std::string program;
		std::size_t pos = 0;
		char current_char;

		lexing_state state;

	public:
		lexer(const std::string &program)
			: program(program)
			, current_char(program.empty() ? '\0' : program[0])
		{
		}

		void consume()
		{
			pos++;
			current_char = (pos < program.length()) ? program[pos] : '\0';
		}

		token get_next_token()
		{
			while (current_char != '\0')
			{
				if (std::isspace(current_char))
				{
					skip_whitespace();
					continue;
				}

				if (std::isdigit(current_char))
				{
					return get_number_token();
				}

				if (std::isalpha(current_char))
				{
					return get_identifier_token();
				}

				if (current_char == '"')
				{
					return get_string_token();
				}

				auto token = char_tokens.find(std::string(1, current_char));
				if (token != char_tokens.end())
				{
					struct token result;

					result.type = token->second;
					result.value = token->first;

					consume();
					return result;
				}

				throw std::runtime_error("Unexpected character: " + std::string(1, current_char));
			}

			return { token_type::eof, "" };
		}

		let_data lex_let()
		{
			auto [identifier_token, type_token] = handle_type_declaration(get_next_token());
			expect_token(token_type::equals, get_next_token());

			auto value_expr = parse_expression();

			type_data type_data {
				.type = type_token.type,
				.type_name = type_token.value
			};

			var_data var_data {
				.type_container = type_data,
				.name = identifier_token.value
			};

			return let_data {
				.var = var_data,
				.value = value_expr->value,
			};
		}

		lex::function_data lex_function()
		{
			state = lexing_state::function_sig;

			token identifier_token = get_next_token();
			expect_token(token_type::identifier, identifier_token);
			expect_token(token_type::left_parenthesis, get_next_token());

			token current;
			function_data data {
				.function_name = identifier_token.value
			};

			while ((current = get_next_token()).type != token_type::right_parenthesis)
			{
				auto [ident_token, type_token] = handle_type_declaration(current);
				auto mapped_token = type_tokens.find(type_token.value);

				if (mapped_token != type_tokens.end())
				{
					var_data var_data = {
						.name = current.value,
					};
					var_data.type_container = type_data {
						.type = mapped_token->second,
						.type_name = type_token.value,
					};

					data.token_data.push_back(var_data);
					data.token_data_index.emplace(current.value, data.token_data_index.size());
				}

				auto next = get_next_token();

				if (next.type == token_type::right_parenthesis)
				{
					break;
				}
				else if (next.type != token_type::comma)
				{
					throw std::runtime_error("expected comma, found " + next.value);
				}
			}

			state = lexing_state::function_body;
			try
			{
				expect_token(token_type::left_brace, get_next_token());
				int begin_address = get_token_pos() + 1;

				token current;
				while ((current = get_next_token()).type != token_type::right_brace)
				{
					if (current.type == token_type::eof)
					{
						throw std::runtime_error("reached EOF before ending function, missing left brace?");
					}
				}

				int end_address = get_token_pos() - 2;
				data.address = std::make_pair(begin_address, end_address);
			} catch (std::exception &e)
			{
				throw std::runtime_error("no function body: " + std::string(e.what()));
			}

			state = lexing_state::unknown;
			return data;
		}

		function_call_data lex_function_call(const token &current, std::map<std::string, std::map<std::string, token_type>> &function_token_map)
		{
			state = lexing_state::function_call;

			function_call_data call_data;
			call_data.function_name = current.value;

			auto tokens = function_token_map.find(current.value);

			expect_token(token_type::left_parenthesis, get_next_token());

			if (tokens != function_token_map.end())
			{
				for (auto entry : tokens->second)
				{
					if (get_at() == ',')
					{
						consume();
					}

					var_data var_data {
						.name = entry.first,
					};

					var_data.type_container.type = entry.second;
					var_data.type_container.type_name = entry.first;

					let_data let_data {
						.var = var_data,
						.value = get_next_token().value,
					};

					call_data.token_data.push_back(let_data);
				}
			}

			expect_token(token_type::right_parenthesis, get_next_token());

			return call_data;
		}

		std::pair<token, token> handle_type_declaration(token identifier_token)
		{
			expect_token(token_type::identifier, identifier_token);
			expect_token(token_type::colon, get_next_token());

			token type_token = get_next_token();
			expect_token(token_type::identifier, type_token);

			return std::make_pair(identifier_token, type_token);
		}

		char get_at(int index = -1)
		{
			return index == -1 ? current_char : program[index];
		}

		int get_token_pos()
		{
			return pos;
		}

		void set_token_pos(int pos)
		{
			this->pos = pos;
			this->current_char = program[pos];
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

		std::shared_ptr<expression> parse_expression()
		{
			token current_token = get_next_token();

			if (current_token.type == token_type::integer || current_token.type == token_type::string)
			{
				return std::make_shared<expression>(expression {
					.type = expr_type::literal,
					.value = current_token.value });
			}

			if (current_token.type == token_type::identifier)
			{
				return std::make_shared<expression>(expression {
					.type = expr_type::identifier,
					.value = current_token.value });
			}

			if (current_token.type == token_type::identifier && get_at() == '=')
			{
				consume();
				std::shared_ptr<expression> right_expr = parse_expression();
				return std::make_shared<expression>(expr_type::assignment, current_token.value, right_expr);
			}

			if (operators.find(current_token.type) != operators.end())
			{
				auto left_expr = std::make_shared<expression>(expr_type::binary_operator, operators.at(current_token.type));

				left_expr->left = parse_expression();
				left_expr->right = parse_expression();

				return left_expr;
			}

			throw std::runtime_error("Invalid expression");
		}

	private:
		void
		skip_whitespace()
		{
			while (std::isspace(current_char))
			{
				consume();
			}
		}

		token get_number_token()
		{
			std::string number;
			while (std::isdigit(current_char))
			{
				number += current_char;
				consume();
			}
			return { token_type::integer, number };
		}

		token get_identifier_token()
		{
			std::string identifier;

			// is there a better way to do this? looks kinda ugly
			while (std::isalnum(current_char) || current_char == '_')
			{
				identifier += current_char;
				consume();
			}

			auto keyword = keywords.find(identifier);

			if (keyword != keywords.end())
			{
				return { keyword->second, identifier };
			}

			if (current_char == '(' && state != lexing_state::function_sig)
			{
				return { token_type::function_call, identifier };
			}

			return { token_type::identifier, identifier };
		}

		token get_string_token()
		{
			std::string str;
			consume(); // Consume the opening quotation mark

			while (current_char != '\0' && current_char != '"')
			{
				str += current_char;
				consume();
			}

			if (current_char != '"')
			{
				throw std::runtime_error("Unterminated string literal");
			}

			consume(); // Consume the closing quotation mark

			return { token_type::string, str };
		}
	};
}
