#pragma once
#include <string>
namespace AST {
struct INode {
	virtual ~INode()
	{}
};

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
INode *makeExprBinop(INode *binop, INode *lhs, INode *rhs);
INode *makeExprAssign(INode *id, INode *val);
INode *makeExprUnop(INode *unop, INode *expr);

INode *makeBinOpMul();
INode *makeBinOpDiv();
INode *makeBinOpMod();
INode *makeBinOpPlus();
INode *makeBinOpMinus();
INode *makeBinOpLess();
INode *makeBinOpGrtr();
INode *makeBinOpLessOrEq();
INode *makeBinOpGrtrOrEq();
INode *makeBinOpEqual();
INode *makeBinOpNotEqual();
INode *makeBinOpAnd();
INode *makeBinOpOr();

INode *makeUnOpPlus();
INode *makeUnOpMinus();
INode *makeUnOpNot();
INode *makeUnOpPrint();

INode *makeExprApply(INode *id, INode *ops);
INode *makeExprFunc(INode *scope, INode *declist, INode *id = nullptr);
INode *makeDeclList(INode *declist, INode *id);
INode *makeDeclListTerm();
INode *makeExprList(INode *exprlist, INode *expr);
INode *makeExprListTerm();
}
