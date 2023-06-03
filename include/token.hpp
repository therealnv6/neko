#pragma once
#include <cwchar>
#include <string>
#include <unordered_map>

enum class token_type
{
	let,
	ident,
	colon,
	integer,
	eq,
	add,
	minus,
	asterisk,
	slash,
	print,
	str,
	eof
};

struct token {
	token_type type;
	std::string value;
};

static std::unordered_map<std::string, token_type> type_tokens = {
	{	  "str",	 token_type::str},
	{	  "int", token_type::integer},
	{"integer", token_type::integer},
};

static std::unordered_map<std::string, token_type> char_tokens = {
	{":",	   token_type::colon},
	{"=",	   token_type::eq},
	{"+",	   token_type::add},
	{"-",	   token_type::minus},
	{"*", token_type::asterisk},
	{"/",	   token_type::slash},
};
;
