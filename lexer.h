#pragma once

#ifndef yyFlexLexerOnce
#include <FlexLexer.h>
#endif

#undef	YY_DECL
#define	YY_DECL	\
	yy::parser::token_kind_type yy::Lexer::yylex(yy::parser::semantic_type *yylval, \
			yy::location *yyloc)


namespace yy {
class Lexer : public yyFlexLexer
{
	public:
	using FlexLexer::yylex;
	yy::parser::token_kind_type yylex(yy::parser::semantic_type *yylval,
			yy::location *yyloc);
	Lexer(std::istream *is)
	{}
};
}
