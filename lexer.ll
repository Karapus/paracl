%option c++
%option noyywrap nodefault
%{
	#include "grammar.tab.hh"
	#include "lexer.h"
	#include <string>
	#define YY_USER_ACTION yyloc->columns(YYLeng());
	#define YY_TERMINATE return 
%}
%option yyclass="yy::Lexer"

wc	[ \t\r]
num	([1-9][0-9]*)|"0"
id	[a-zA-Z_][a-zA-Z_0-9]*

%%

%{
	yyloc->step();
%}

{wc}+	yyloc->step();
\n+	yyloc->lines(YYLeng()); yyloc->step();
"+"		return yy::parser::token::TOK_PLUS;
"-"		return yy::parser::token::TOK_MINUS;
"*"		return yy::parser::token::TOK_STAR;
"/"		return yy::parser::token::TOK_SLASH;
"=="		return yy::parser::token::TOK_EQ;
"!="		return yy::parser::token::TOK_NEQ;
"<="		return yy::parser::token::TOK_LE;
">="		return yy::parser::token::TOK_GE;
"="		return yy::parser::token::TOK_ASSIGN;
"!"		return yy::parser::token::TOK_EXCL;
"<"		return yy::parser::token::TOK_LT;
">"		return yy::parser::token::TOK_GT;
"?"		return yy::parser::token::TOK_QMARK;
"print"		return yy::parser::token::TOK_PRINT;
"while" 	return yy::parser::token::TOK_WHILE;
"if"		return yy::parser::token::TOK_IF;
"else"		return yy::parser::token::TOK_ELSE;
"func"		return yy::parser::token::TOK_FUNC;
"return"	return yy::parser::token::TOK_RETURN;
"{"		return yy::parser::token::TOK_LBRACE;
"}"		return yy::parser::token::TOK_RBRACE;
"("		return yy::parser::token::TOK_LPAR;
")"		return yy::parser::token::TOK_RPAR;
";"		return yy::parser::token::TOK_SEMICOLON;
":"		return yy::parser::token::TOK_COLON;
","		return yy::parser::token::TOK_COMA;
{num}	{
		*yylval = AST::makeExprInt(std::stoi(YYText()));
		return yy::parser::token::TOK_NUM;
	}
{id}	{
		*yylval = AST::makeExprId(YYText());
		return yy::parser::token::TOK_ID;
	}
.	throw yy::parser::syntax_error(*yyloc, "invalid character: " + std::string(YYText()));
<<EOF>>	return yy::parser::token::TOK_END;
%%
