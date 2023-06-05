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
	multiply,
	divide,
	comma,

	// Functions
	function,
	function_call,
	parameter,
	left_parenthesis,
	right_parenthesis,
	left_brace,
	right_brace,
	left_square_bracket,
	right_square_bracket,

	// End of file
	eof
};

static const std::unordered_map<std::string, token_type> char_tokens = {
	{":",				token_type::colon},
	{"=",			   token_type::equals},
	{"+",				 token_type::plus},
	{"-",				token_type::minus},
	{"*",			 token_type::multiply},
	{"/",			   token_type::divide},
	{"(",	   token_type::left_parenthesis},
	{")",	   token_type::right_parenthesis},
	{"{",		   token_type::left_brace},
	{"}",		   token_type::right_brace},
	{"[",  token_type::left_square_bracket},
	{"]", token_type::right_square_bracket},
	{",",				token_type::comma},
};

static const std::unordered_map<std::string, token_type> keywords = {
	{  "let",		 token_type::let},
	{"print",	   token_type::print},
	{	  "fn", token_type::function},
};

static const std::unordered_map<std::string, token_type> type_tokens = {
	{"int", token_type::integer},
	{"str",	 token_type::string}
};

static const std::unordered_map<token_type, std::string> operators = {
	{	  token_type::plus, "+"},
	{	  token_type::minus, "-"},
	{token_type::multiply, "*"},
	{  token_type::divide, "/"},
};

enum class expr_type
{
	literal,
	identifier,
	assignment,
	binary_operator,
};

struct token {
	token_type type;
	std::string value;
};
