%language "c++"
%require "3.2"
%defines
%locations

%code requires {
	namespace yy {
		class Driver;
	}
	#include "inode.hh"
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
%define parse.error verbose
%define parse.lac full

%define api.token.prefix {TOK_}
%token
	END	0
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
	FUNC
	LBRACE
	RBRACE
	LPAR
	RPAR
	SEMICOLON
	COLON
	COMA
	NUM
	ID
	FUNCTION
	RETURN

%destructor { delete $$; } NUM ID scope stm expr unop

%right ELSE THEN
%right ASSIGN
%left EQ NEQ LE GE LT GT
%left PLUS MINUS
%left STAR SLASH
%precedence UNOP

%start program
%%
program : scope	END	{ driver.yylval = $1; }
;

scope   : blocks        { $$ = AST::makeScope($1);      }
;

blocks	: blocks block	{ $$ = AST::makeBlocks($1, $2); }
        | %empty	{ $$ = AST::makeBlocksTerm();   }
        | error		{ $$ = AST::makeBlocksTerm();   }
;

block	: stm			{ $$ = $1; }
	| LBRACE scope RBRACE	{ $$ = $2; }
;

stm	: SEMICOLON					{ $$ = AST::makeBlocksTerm();		}
	| expr SEMICOLON				{ $$ = AST::makeStmExpr($1);		}
	| PRINT expr SEMICOLON				{ $$ = AST::makeStmPrint($2);		}
	| WHILE LPAR expr RPAR block			{ $$ = AST::makeStmWhile($3, $5);	}
	| IF LPAR expr RPAR block 	%prec THEN	{ $$ = AST::makeStmIf($3, $5);		}
	| IF LPAR expr RPAR block ELSE block		{ $$ = AST::makeStmIf($3, $5, $7);	}
;

expr	: LPAR expr RPAR	{ $$ = $2; 				}
	| ID ASSIGN expr	{ $$ = AST::makeExprAssign($1, $3);	}
	| ID applist		{ $$ = AST::makeExprApply($1, $2);	}
	| unop expr %prec UNOP	{ $$ = AST::makeExprUnop($1, $2);	}
	| NUM			{ $$ = $1;				}
	| ID			{ $$ = $1;				}
	| func			{ $$ = $1;				}
	| QMARK			{ $$ = AST::makeExprQmark();		}
	| expr STAR	{ $$ = AST::makeBinOpMul();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| expr SLASH	{ $$ = AST::makeBinOpDiv();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| expr PLUS	{ $$ = AST::makeBinOpPlus();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| expr MINUS	{ $$ = AST::makeBinOpMinus();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| expr LT	{ $$ = AST::makeBinOpLess();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| expr GT	{ $$ = AST::makeBinOpGrtr();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| expr LE	{ $$ = AST::makeBinOpLessOrEq();} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| expr GE	{ $$ = AST::makeBinOpGrtrOrEq();} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| expr EQ	{ $$ = AST::makeBinOpEqual();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| expr NEQ	{ $$ = AST::makeBinOpNotEqual();} expr { $$ = AST::makeExprBinop($3, $1, $4); }
;

unop	: PLUS		{ $$ = AST::makeUnOpPlus();	}
	| MINUS		{ $$ = AST::makeUnOpMinus();	}
	| EXCL		{ $$ = AST::makeUnOpNot();	}
;

func	: FUNC declist LBRACE scope RBRACE		{ $$ = AST::makeFunc($4, $2);		}
	| FUNC declist COLON ID LBRACE scope RBRACE	{ $$ = AST::makeFunc($6, $2, $4);	}
;

declist	: LPAR decls RPAR	{ $$ = $2;			}
	| LPAR RPAR		{ $$ = AST::makeDeclistTerm();	}
;

decls	: decls COMA ID	{ $$ = AST::makeDeclist($1, $3);		}
	| ID		{ $$ = AST::makeDeclist(AST::makeDeclistTerm(), $1);	}
;

applist	: LPAR exprs RPAR	{ $$ = $2;			}
	| LPAR RPAR		{ $$ = AST::makeExprlistTerm();	}
;

exprs	: exprs COMA expr	{ $$ = AST::makeExprlist($1, $3);			}
	| expr			{ $$ = AST::makeExprlist(AST::makeExprlistTerm(), $1);	}
;

%%

void yy::parser::error(const location_type &loc, const std::string &err_message) {
	std::cerr << "Error: " << err_message << " at " << loc << std::endl;
}
