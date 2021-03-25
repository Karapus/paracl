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
	#include "driver.hh"
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
	PERCNT
	EQ
	ASSIGN
	EXCL
	NEQ
	LE
	GE
	LT
        GT
	AND
	OR
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
	COLON
	COMA
	NUM
	ID
	FUNC
	RETURN

%destructor { delete $$; } ID NUM scope blocks block
	stm cexpr fexpr expr unop func declist decls
	applist exprs

%right ELSE THEN
%precedence PRINT RETURN
%left OR
%left AND
%precedence ASSIGN
%left EQ NEQ LE GE LT GT
%left PLUS MINUS
%left STAR SLASH PERCNT
%precedence UNOP

%start program
%%
program : blocks END	{ driver.yylval = AST::makeScope($1);	}
;

scope   : LBRACE blocks RBRACE 	{ $$ = AST::makeScope($2);	}
;

blocks	: blocks block		{ $$ = AST::makeSeq($1, $2);	}
	| %empty		{ $$ = AST::makeEmpty();		}
;

block	: stm			{ $$ = $1;	}
	| fexpr SEMICOLON	{ $$ = $1;	}
	| SEMICOLON		{ $$ = AST::makeEmpty();	}
        | error 		{ $$ = AST::makeEmpty();	}
;

stm	: scope						{ $$ = $1; }
	| WHILE LPAR expr RPAR block			{ $$ = AST::makeWhile($3, $5);	}
	| IF LPAR expr RPAR block ELSE block		{ $$ = AST::makeIf($3, $5, $7);	}
	| IF LPAR expr RPAR block	%prec THEN	{ $$ = AST::makeIf($3, $5);	}
	| ID ASSIGN func		 		{ $$ = AST::makeExprAssign($1, $3);	}
;

cexpr	: LPAR expr RPAR	{ $$ = $2; 	}
	| NUM			{ $$ = $1;	}
	| ID			{ $$ = $1;	}
	| QMARK			{ $$ = AST::makeExprQmark();		}
	| ID ASSIGN expr	{ $$ = AST::makeExprAssign($1, $3);	}
	| ID applist		{ $$ = AST::makeExprApply($1, $2);	}
	| RETURN expr		{ $$ = AST::makeReturn($2);		}
	| PRINT expr		{ $$ = AST::makeExprUnop(AST::makeUnOpPrint(), $2);	}
	| unop expr	%prec UNOP	{ $$ = AST::makeExprUnop($1, $2);		}
;

fexpr	: cexpr		{$$ = $1; }
	| fexpr PLUS	{ $$ = AST::makeBinOpPlus();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| fexpr MINUS	{ $$ = AST::makeBinOpMinus();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| fexpr STAR	{ $$ = AST::makeBinOpMul();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| fexpr SLASH	{ $$ = AST::makeBinOpDiv();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| fexpr PERCNT	{ $$ = AST::makeBinOpMod();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| fexpr LT	{ $$ = AST::makeBinOpLess();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| fexpr GT	{ $$ = AST::makeBinOpGrtr();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| fexpr LE	{ $$ = AST::makeBinOpLessOrEq();} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| fexpr GE	{ $$ = AST::makeBinOpGrtrOrEq();} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| fexpr EQ	{ $$ = AST::makeBinOpEqual();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| fexpr NEQ	{ $$ = AST::makeBinOpNotEqual();} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| fexpr AND	{ $$ = AST::makeBinOpAnd();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| fexpr OR	{ $$ = AST::makeBinOpOr();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
;

expr	: cexpr 	{ $$ = $1;	}
	| stm		{ $$ = $1;	}
	| expr PLUS	{ $$ = AST::makeBinOpPlus();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| expr MINUS	{ $$ = AST::makeBinOpMinus();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| expr STAR	{ $$ = AST::makeBinOpMul();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| expr SLASH	{ $$ = AST::makeBinOpDiv();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| expr PERCNT	{ $$ = AST::makeBinOpMod();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| expr LT	{ $$ = AST::makeBinOpLess();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| expr GT	{ $$ = AST::makeBinOpGrtr();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| expr LE	{ $$ = AST::makeBinOpLessOrEq();} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| expr GE	{ $$ = AST::makeBinOpGrtrOrEq();} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| expr EQ	{ $$ = AST::makeBinOpEqual();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| expr NEQ	{ $$ = AST::makeBinOpNotEqual();} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| expr AND	{ $$ = AST::makeBinOpAnd();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
	| expr OR	{ $$ = AST::makeBinOpOr();	} expr { $$ = AST::makeExprBinop($3, $1, $4); }
;

unop	: PLUS		{ $$ = AST::makeUnOpPlus();	}
	| MINUS		{ $$ = AST::makeUnOpMinus();	}
	| EXCL		{ $$ = AST::makeUnOpNot();	}
;

func	: FUNC declist scope		{ $$ = AST::makeExprFunc($3, $2);	}
	| FUNC declist COLON ID scope	{ $$ = AST::makeExprFunc($5, $2, $4);	}
;

declist	: LPAR decls RPAR	{ $$ = $2;			}
	| LPAR RPAR		{ $$ = AST::makeDeclListTerm();	}
;

decls	: decls COMA ID	{ $$ = AST::makeDeclList($1, $3);			}
	| ID		{ $$ = AST::makeDeclList(AST::makeDeclListTerm(), $1);	}
;

applist	: LPAR exprs RPAR	{ $$ = $2;			}
	| LPAR RPAR		{ $$ = AST::makeExprListTerm();	}
;

exprs	: exprs COMA expr	{ $$ = AST::makeExprList($1, $3);			}
	| expr			{ $$ = AST::makeExprList(AST::makeExprListTerm(), $1);	}
;

%%

void yy::parser::error(const location_type &loc, const std::string &err_message) {
	std::cerr << "Error: " << err_message << " at " << loc << std::endl;
}
