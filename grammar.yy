%language "c++"
%require "3.2"

%code requires {
	namespace yy {
		class Lexer;
	}
	#include "inode.h"
}

%define api.value.type {AST::INode *}
%parse-param { Lexer &lexer }

%code {
	#include "lexer.h"
	#undef	yylex
	#define	yylex lexer.yylex
}

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
	LBRACE
	RBRACE
	LPAR
	RPAR
	SEMICOLON
%token NUM
%token ID

%right ASSIGN
%left EQ NEQ LE GE LT GT
%left PLUS MINUS
%left STAR SLASH
%precedence UNOP EXCL

%start scope
%%

scope	: blocks	{ $$ = AST::makeScope($1);	}

blocks	: block blocks	{ $$ = AST::makeBlocks($1, $2);	}
	| %empty	{ $$ = AST::makeBlockTerm(); 	}
;

block	: stm SEMICOLON		{ $$ = $1;			}
	| LBRACE scope RBRACE	{ $$ = AST::makeScope($2);	}
;

stm	: expr				{ $$ = AST::makeStmExpr($1);		}
	| PRINT expr			{ $$ = AST::makeStmPrint($2);		}
	| WHILE LPAR expr RPAR block	{ $$ = AST::makeStmWhile($3, $5);	}
;

expr	: LPAR expr RPAR	{ $$ = $2; 				}
	| expr binop expr	{ $$ = AST::makeExprBinop($2, $1, $3);	}
	| ID ASSIGN expr	{ $$ = AST::makeExprAssign($1, $3);	}
	| unop expr		{ $$ = AST::makeExprUnop($1, $2);	}
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

unop	: PLUS	%prec UNOP	{ $$ = AST::makeUnOpPlus();	}
	| MINUS	%prec UNOP	{ $$ = AST::makeUnOpMinus();	}
	| EXCL			{ $$ = AST::makeUnOpNot();	}
;
%%

void yy::parser::error(const std::string &err_message) {
	std::cerr << "Error: " << err_message << std::endl;
}
