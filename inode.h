#pragma once
#include <string>
namespace AST {
class INode {
public:
	virtual ~INode()
	{}
};

INode *makeExprInt(int num);
INode *makeExprId(std::string name);

INode *makeScope(INode *blocks);

INode *makeBlocks(INode *head, INode *tail);
INode *makeBlocksTerm();

INode *makeStmExpr(INode *expr);
INode *makeStmPrint(INode *expr);
INode *makeStmWhile(INode *expr, INode *scope);
INode *makeStmIf(INode *expr, INode *true_block, INode *false_block = nullptr);

INode *makeExprBinop(INode *binop, INode *lhs, INode *rhs);
INode *makeExprAssign(INode *id, INode *val);
INode *makeExprUnop(INode *unop, INode *expr);
INode *makeExprQmark();

INode *makeBinOpMul();
INode *makeBinOpDiv();
INode *makeBinOpPlus();
INode *makeBinOpMinus();
INode *makeBinOpLess();
INode *makeBinOpGrtr();
INode *makeBinOpLessOrEq();
INode *makeBinOpGrtrOrEq();
INode *makeBinOpEqual();
INode *makeBinOpNotEqual();

INode *makeUnOpPlus();
INode *makeUnOpMinus();
INode *makeUnOpNot();

}
