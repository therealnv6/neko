#pragma once
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <token.hpp>
#include <unordered_map>

namespace lex
{
	class lexer
	{
	private:
		std::string program;
		std::size_t pos = 0;
		char current_char = '\0';

	public:
		lexer(const std::string &program)
			: program(program)
		{
			current_char = program.empty() ? '\0' : program[0];
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

					consume_char();
					return result;
				}

				throw std::runtime_error("Unexpected character: " + std::string(1, current_char));
			}

			return { token_type::eof, "" };
		}

	private:
		void consume_char()
		{
			pos++;
			current_char = (pos < program.length()) ? program[pos] : '\0';
		}

		void skip_whitespace()
		{
			while (std::isspace(current_char))
			{
				consume_char();
			}
		}

		token get_number_token()
		{
			std::string number;
			while (std::isdigit(current_char))
			{
				number += current_char;
				consume_char();
			}
			return { token_type::integer, number };
		}

		token get_identifier_token()
		{
			std::string identifier;
			while (std::isalnum(current_char))
			{
				identifier += current_char;
				consume_char();
			}

			auto keyword = keywords.find(identifier);
			if (keyword != keywords.end())
			{
				return { keyword->second, identifier };
			}

			return { token_type::identifier, identifier };
		}

		token get_string_token()
		{
			std::string str;
			consume_char(); // Consume the opening quotation mark

			while (current_char != '\0' && current_char != '"')
			{
				str += current_char;
				consume_char();
			}

			if (current_char != '"')
			{
				throw std::runtime_error("Unterminated string literal");
			}

			consume_char(); // Consume the closing quotation mark

			return { token_type::string, str };
		}
	};
}
