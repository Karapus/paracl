%language "c++"
%require "3.2"
%defines
%locations

%code requires {
	namespace yy {
		class Driver;
	}
	#include "inode.h"
}

%define api.token.raw
%define api.value.type {AST::INode *}
%parse-param { Driver &driver }

%code {
	#include "driver.h"
	#undef	yylex
	#define	yylex driver.lexer.yylex
}

%define parse.trace
%define parse.error detailed
%define parse.lac full

%define api.token.prefix {TOK_}
%token
	PLUS
	MINUS
	STAR
	SLASH
	EQ
	ASSIGN
	EXCL
	NEQ
	LE
	GE
	LT
        GT
	QMARK
	PRINT
	WHILE
	IF
	ELSE
	LBRACE
	RBRACE
	LPAR
	RPAR
	SEMICOLON
	NUM
	ID
;

/*%right RBRACE ELSE*/
%right ASSIGN
%left EQ NEQ LE GE LT GT
%left PLUS MINUS
%left STAR SLASH
%precedence UNOP

%start program
%%
program : scope		{ driver.yylval = $1; }
;

scope	: blocks	{ $$ = AST::makeScope($1);	}
;

blocks	: blocks block	{ $$ = AST::makeBlocks($1, $2);	}
	| %empty	{ $$ = AST::makeBlocksTerm(); 	}
;

block	: stm			{ $$ = $1;			}
	| LBRACE scope RBRACE	{ $$ = AST::makeScope($2);	}
;

stm	: expr	SEMICOLON			{ $$ = AST::makeStmExpr($1);		}
	| PRINT expr SEMICOLON			{ $$ = AST::makeStmPrint($2);		}
	| WHILE LPAR expr RPAR block		{ $$ = AST::makeStmWhile($3, $5);	}
	| IF LPAR expr RPAR block		{ $$ = AST::makeStmIf($3, $5);		}
	| IF LPAR expr RPAR block ELSE block	{ $$ = AST::makeStmIf($3, $5, $7);	}
;

expr	: LPAR expr RPAR	{ $$ = $2; 				}
	| expr binop expr 	{ $$ = AST::makeExprBinop($2, $1, $3);	}
	| ID ASSIGN expr	{ $$ = AST::makeExprAssign($1, $3);	}
	| unop expr %prec UNOP	{ $$ = AST::makeExprUnop($1, $2);	}
	| NUM			{ $$ = $1;				}
	| ID			{ $$ = $1;				}
	| QMARK			{ $$ = AST::makeExprQmark();		}
;

binop	: STAR		{ $$ = AST::makeBinOpMul();		}
	| SLASH		{ $$ = AST::makeBinOpDiv();		}
	| PLUS		{ $$ = AST::makeBinOpPlus();		}
	| MINUS		{ $$ = AST::makeBinOpMinus();		}
	| LT		{ $$ = AST::makeBinOpLess();		}
	| GT		{ $$ = AST::makeBinOpGrtr();		}
	| LE		{ $$ = AST::makeBinOpLessOrEq();	}
	| GE		{ $$ = AST::makeBinOpGrtrOrEq();	}
	| EQ		{ $$ = AST::makeBinOpEqual();		}
	| NEQ		{ $$ = AST::makeBinOpNotEqual();	}
;

unop	: PLUS		{ $$ = AST::makeUnOpPlus();	}
	| MINUS		{ $$ = AST::makeUnOpMinus();	}
	| EXCL		{ $$ = AST::makeUnOpNot();	}
;
%%

void yy::parser::error(const location_type &loc, const std::string &err_message) {
	std::cerr << "Error: " << err_message << " at " << loc << std::endl;
}
