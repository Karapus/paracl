#pragma once
#include <string>
#include "ast.hh"
namespace AST {

INode *makeExprInt(int num);
INode *makeExprId(std::string name);
INode *makeExprQmark();
INode *makeEmpty();

INode *makeScope(INode *blocks);

INode *makePrint(INode *expr);
INode *makeWhile(INode *expr, INode *scope);
INode *makeIf(INode *expr, INode *true_block, INode *false_block = nullptr);
INode *makeReturn(INode *expr);

INode *makeSeq(INode *fst, INode *snd);
INode *makeExprAssign(INode *id, INode *val);

template <typename T>
INode *makeExprBinop(INode *lhs, INode *rhs) {
	return new ExprBinOp<T>{static_cast<Expr *>(lhs), static_cast<Expr *>(rhs)};
}

template <typename T>
INode *makeExprUnop(INode *expr) {
	return new ExprUnOp<T>{static_cast<Expr *>(expr)};
}

INode *makeExprApply(INode *id, INode *ops);
INode *makeExprFunc(INode *scope, INode *declist, INode *id = nullptr);
INode *makeDeclList(INode *declist, INode *id);
INode *makeDeclListTerm();
INode *makeExprList(INode *exprlist, INode *expr);
INode *makeExprListTerm();
}
