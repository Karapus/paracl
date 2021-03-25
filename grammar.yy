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
	stm cexpr fexpr expr func declist decls
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
	| PRINT expr		{ $$ = AST::makeExprUnop<AST::UnOpPrint	>($2);	}
	| PLUS	expr %prec UNOP	{ $$ = AST::makeExprUnop<AST::UnOpPlus	>($2);	}
	| MINUS	expr %prec UNOP	{ $$ = AST::makeExprUnop<AST::UnOpMinus	>($2);	}
	| EXCL	expr %prec UNOP	{ $$ = AST::makeExprUnop<AST::UnOpNot	>($2);	}
;

;

fexpr	: cexpr		{$$ = $1; }
	| fexpr PLUS	expr	{ $$ = AST::makeExprBinop<AST::BinOpPlus	>($1, $3);	}
	| fexpr MINUS	expr	{ $$ = AST::makeExprBinop<AST::BinOpMinus	>($1, $3);	}
	| fexpr STAR	expr	{ $$ = AST::makeExprBinop<AST::BinOpMul		>($1, $3);	}
	| fexpr SLASH	expr	{ $$ = AST::makeExprBinop<AST::BinOpDiv		>($1, $3);	}
	| fexpr PERCNT	expr	{ $$ = AST::makeExprBinop<AST::BinOpMod		>($1, $3);	}
	| fexpr LT	expr	{ $$ = AST::makeExprBinop<AST::BinOpLess	>($1, $3);	}
	| fexpr GT	expr	{ $$ = AST::makeExprBinop<AST::BinOpGrtr	>($1, $3);	}
	| fexpr LE	expr	{ $$ = AST::makeExprBinop<AST::BinOpLessOrEq	>($1, $3);	}
	| fexpr GE	expr	{ $$ = AST::makeExprBinop<AST::BinOpGrtrOrEq	>($1, $3);	}
	| fexpr EQ	expr	{ $$ = AST::makeExprBinop<AST::BinOpEqual	>($1, $3);	}
	| fexpr NEQ	expr	{ $$ = AST::makeExprBinop<AST::BinOpNotEqual	>($1, $3);	}
	| fexpr AND	expr	{ $$ = AST::makeExprBinop<AST::BinOpAnd		>($1, $3);	}
	| fexpr OR	expr	{ $$ = AST::makeExprBinop<AST::BinOpOr		>($1, $3);	}
;

expr	: cexpr 	{ $$ = $1;	}
	| stm		{ $$ = $1;	}
	| expr PLUS	expr	{ $$ = AST::makeExprBinop<AST::BinOpPlus	>($1, $3);	}
	| expr MINUS	expr	{ $$ = AST::makeExprBinop<AST::BinOpMinus	>($1, $3);	}
	| expr STAR	expr	{ $$ = AST::makeExprBinop<AST::BinOpMul		>($1, $3);	}
	| expr SLASH	expr	{ $$ = AST::makeExprBinop<AST::BinOpDiv		>($1, $3);	}
	| expr PERCNT	expr	{ $$ = AST::makeExprBinop<AST::BinOpMod		>($1, $3);	}
	| expr LT	expr	{ $$ = AST::makeExprBinop<AST::BinOpLess	>($1, $3);	}
	| expr GT	expr	{ $$ = AST::makeExprBinop<AST::BinOpGrtr	>($1, $3);	}
	| expr LE	expr	{ $$ = AST::makeExprBinop<AST::BinOpLessOrEq	>($1, $3);	}
	| expr GE	expr	{ $$ = AST::makeExprBinop<AST::BinOpGrtrOrEq	>($1, $3);	}
	| expr EQ	expr	{ $$ = AST::makeExprBinop<AST::BinOpEqual	>($1, $3);	}
	| expr NEQ	expr	{ $$ = AST::makeExprBinop<AST::BinOpNotEqual	>($1, $3);	}
	| expr AND	expr	{ $$ = AST::makeExprBinop<AST::BinOpAnd		>($1, $3);	}
	| expr OR	expr	{ $$ = AST::makeExprBinop<AST::BinOpOr		>($1, $3);	}
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
