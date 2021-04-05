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
	using namespace AST;
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
program : blocks END	{ driver.yylval = make<Scope>(@$, $1);	}
;

scope   : LBRACE blocks RBRACE 	{ $$ = make<Scope>(@$, $2);	}
;

blocks	: blocks block		{ $$ = make<Seq>(@$, $1, $2);	}
	| %empty		{ $$ = make<Empty>(@$);		}
;

block	: stm			{ $$ = $1;	}
	| fexpr SEMICOLON	{ $$ = $1;	}
	| SEMICOLON		{ $$ = make<Empty>(@$);	}
        | error 		{ $$ = make<Empty>(@$);	}
;

stm	: scope						{ $$ = $1;	}
	| WHILE LPAR expr RPAR block			{ $$ = make<While>(@$, $3, $5);		}
	| IF LPAR expr RPAR block ELSE block		{ $$ = make<If>(@$, $3, $5, $7);	}
	| IF LPAR expr RPAR block	%prec THEN	{ $$ = make<If>(@$, $3, $5);		}
	| ID ASSIGN func		 		{ $$ = make<ExprAssign>(@$, $1, $3);	}
;

cexpr	: LPAR aexpr RPAR	{ $$ = $2; 	}
	| NUM			{ $$ = $1;	}
	| ID			{ $$ = $1;	}
	| QMARK			{ $$ = make<ExprQmark>(@$);			}
	| RETURN expr		{ $$ = make<Return>(@$, $2);			}
	| ID applist		{ $$ = make<ExprApply>(@$, $1, $2);		}
	| PRINT expr		{ $$ = make<ExprUnOp<UnOpPrint	>>(@$, $2);	}
	| PLUS	expr %prec UNOP	{ $$ = make<ExprUnOp<UnOpPlus	>>(@$, $2);	}
	| MINUS	expr %prec UNOP	{ $$ = make<ExprUnOp<UnOpMinus	>>(@$, $2);	}
	| EXCL	expr %prec UNOP	{ $$ = make<ExprUnOp<UnOpNot	>>(@$, $2);	}
;

aexpr	: ID ASSIGN aexpr	{ $$ = make<ExprAssign>(@$, $1, $3);	}
	| expr	 %prec ASSIGN	{ $$ = $1;	}
;

fexpr	: cexpr			{ $$ = $1;	}
	| ID ASSIGN aexpr	{ $$ = make<ExprAssign>(@$, $1, $3);	}
	| fexpr PLUS	expr	{ $$ = make<ExprBinOp<BinOpPlus		>>(@$, $1, $3);	}
	| fexpr MINUS	expr	{ $$ = make<ExprBinOp<BinOpMinus	>>(@$, $1, $3);	}
	| fexpr STAR	expr	{ $$ = make<ExprBinOp<BinOpMul		>>(@$, $1, $3);	}
	| fexpr SLASH	expr	{ $$ = make<ExprBinOp<BinOpDiv		>>(@$, $1, $3);	}
	| fexpr PERCNT	expr	{ $$ = make<ExprBinOp<BinOpMod		>>(@$, $1, $3);	}
	| fexpr LT	expr	{ $$ = make<ExprBinOp<BinOpLess		>>(@$, $1, $3);	}
	| fexpr GT	expr	{ $$ = make<ExprBinOp<BinOpGrtr		>>(@$, $1, $3);	}
	| fexpr LE	expr	{ $$ = make<ExprBinOp<BinOpLessOrEq	>>(@$, $1, $3);	}
	| fexpr GE	expr	{ $$ = make<ExprBinOp<BinOpGrtrOrEq	>>(@$, $1, $3);	}
	| fexpr EQ	expr	{ $$ = make<ExprBinOp<BinOpEqual	>>(@$, $1, $3);	}
	| fexpr NEQ	expr	{ $$ = make<ExprBinOp<BinOpNotEqual	>>(@$, $1, $3);	}
	| fexpr AND	expr	{ $$ = make<ExprBinOp<BinOpAnd		>>(@$, $1, $3);	}
	| fexpr OR	expr	{ $$ = make<ExprBinOp<BinOpOr		>>(@$, $1, $3);	}
;

expr	: cexpr 		{ $$ = $1;	}
	| stm			{ $$ = $1;	}
	| expr PLUS	expr	{ $$ = make<ExprBinOp<BinOpPlus>>	(@$, $1, $3);	}
	| expr MINUS	expr	{ $$ = make<ExprBinOp<BinOpMinus>>	(@$, $1, $3);	}
	| expr STAR	expr	{ $$ = make<ExprBinOp<BinOpMul>>	(@$, $1, $3);	}
	| expr SLASH	expr	{ $$ = make<ExprBinOp<BinOpDiv>>	(@$, $1, $3);	}
	| expr PERCNT	expr	{ $$ = make<ExprBinOp<BinOpMod>>	(@$, $1, $3);	}
	| expr LT	expr	{ $$ = make<ExprBinOp<BinOpLess>>	(@$, $1, $3);	}
	| expr GT	expr	{ $$ = make<ExprBinOp<BinOpGrtr>>	(@$, $1, $3);	}
	| expr LE	expr	{ $$ = make<ExprBinOp<BinOpLessOrEq>>	(@$, $1, $3);	}
	| expr GE	expr	{ $$ = make<ExprBinOp<BinOpGrtrOrEq>>	(@$, $1, $3);	}
	| expr EQ	expr	{ $$ = make<ExprBinOp<BinOpEqual>>	(@$, $1, $3);	}
	| expr NEQ	expr	{ $$ = make<ExprBinOp<BinOpNotEqual>>	(@$, $1, $3);	}
	| expr AND	expr	{ $$ = make<ExprBinOp<BinOpAnd>>	(@$, $1, $3);	}
	| expr OR	expr	{ $$ = make<ExprBinOp<BinOpOr>>		(@$, $1, $3);	}
;

func	: FUNC declist scope		{ $$ = make<ExprFunc>(@$, $3, $2);	}
	| FUNC declist COLON ID scope	{ $$ = make<ExprFunc>(@$, $5, $2, $4);	}
;

declist	: LPAR decls RPAR	{ $$ = $2;			}
	| LPAR RPAR		{ $$ = make<DeclList>();	}
;

decls	: decls COMA ID	{ $$ = make<DeclList>($1, $3);			}
	| ID		{ $$ = make<DeclList>(make<DeclList>(), $1);	}
;

applist	: LPAR exprs RPAR	{ $$ = $2;			}
	| LPAR RPAR		{ $$ = make<ExprList>();	}
;

exprs	: exprs COMA expr	{ $$ = make<ExprList>(@$, $1, $3);			}
	| expr			{ $$ = make<ExprList>(@$, make<ExprList>(), $1);	}
;

%%

void yy::parser::error(const location_type &loc, const std::string &err_message) {
	std::cerr << "Error: " << err_message << " at " << loc << std::endl;
}
