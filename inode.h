#pragma once
#include <string>
namespace AST {
class INode;

INode *makeExprInt(int num);
INode *makeExprId(std::string name);

INode *makeScopes(INode *stm, INode *stms);
INode *makeScopesTerm();

INode *makeScopeStm(INode *stm);
INode *makeScopeBraces(INode *scope);

INode *makeStmExpr(INode *expr);
INode *makeStmDecl(INode *id, INode *expr);
INode *makeStmPrint(INode *expr);
INode *makeStmWhile(INode *expr, INode *scope);

INode *makeExprBinop(INode *binop, INode *lhs, INode *rhs);
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
INode *makeBinOpAssign();

INode *makeUnOpPlus();
INode *makeUnOpMinus();
INode *makeUnOpNot();

}
