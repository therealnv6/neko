#pragma once
#include <string>

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
