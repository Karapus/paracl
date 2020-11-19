%option c++
%option noyywrap
%{
	#include "grammar.tab.hh"
	#include "lexer.h"
	#include <string>
%}
%option yyclass="yy::Lexer"

num	"-?[1-9][0-9]*"
id	"[a-zA-Z_][a-zA-Z_0-9]*"

%%
"+"	return yy::parser::token::PLUS;
"-"	return yy::parser::token::MINUS;
"*"	return yy::parser::token::STAR;
"/"	return yy::parser::token::SLASH;
"=="	return yy::parser::token::EQ;
"!="	return yy::parser::token::NEQ;
"<="	return yy::parser::token::LE;
">="	return yy::parser::token::GE;
"="	return yy::parser::token::ASSIGN;
"!"	return yy::parser::token::EXCL;
"<"	return yy::parser::token::LT;
">"	return yy::parser::token::GT;
"?"	return yy::parser::token::QMARK;
"print"	return yy::parser::token::PRINT;
"while" return yy::parser::token::WHILE;
"{"	return yy::parser::token::LBRACE;
"}"	return yy::parser::token::RBRACE;
"("	return yy::parser::token::LPAR;
")"	return yy::parser::token::RPAR;
";"	return yy::parser::token::SEMICOLON;
{num}	{
		*yylval = AST::makeExprInt(std::stoi(YYText()));
		return yy::parser::token::NUM;
	}
{id}	{
		*yylval = AST::makeExprId(YYText());
		return yy::parser::token::ID;
	}
<<EOF>>	return yy::parser::token::YYEOF;
%%
