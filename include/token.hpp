#pragma once
#include <cwchar>
#include <string>
#include <unordered_map>

enum class token_type
{
	// Keywords
	let,
	print,

	// Identifiers and literals
	identifier,
	integer,
	string,

	// Operators and punctuation
	colon,
	equals,
	plus,
	minus,

	// End of file
	eof
};

static const std::unordered_map<std::string, token_type> char_tokens = {
	{":",  token_type::colon},
	{"=", token_type::equals},
	{"+",   token_type::plus},
	{"-",  token_type::minus}
};

static const std::unordered_map<std::string, token_type> keywords = {
	{  "let",	 token_type::let},
	{"print", token_type::print}
};

const std::unordered_map<std::string, token_type> type_tokens = {
	{"int", token_type::integer},
	{"str",	 token_type::string}
};

struct token {
	token_type type;
	std::string value;
};
