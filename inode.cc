#include "ast.hh"
#include "inode.hh"
using namespace AST;
INode *AST::makeExprInt(int num) {
	return new ExprInt{num};
}
INode *AST::makeExprId(std::string name) {
	return new ExprId{name};
}

INode *AST::makeScope(INode *blocks) {
	return new Scope{static_cast<Blocks *>(blocks)};
}

INode *AST::makeBlocks(INode *head, INode *tail) {
	return new Blocks{static_cast<Block *>(head), static_cast<Blocks *>(tail)};
}
INode *AST::makeBlocksTerm() {
	return nullptr;
}
INode *AST::makeStmExpr(INode *expr) {
	return new StmExpr{static_cast<Expr *>(expr)};
}
INode *AST::makeStmPrint(INode *expr) {
	return new StmPrint{static_cast<Expr *>(expr)};
}
INode *AST::makeStmWhile(INode *expr, INode *block) {
	return new StmWhile{static_cast<Expr *>(expr), static_cast<Block *>(block)};
}
INode *AST::makeStmIf(INode *expr, INode *true_block, INode *false_block) {
	return new StmIf{static_cast<Expr *>(expr), static_cast<Block *>(true_block), static_cast<Block *>(false_block)};
}

INode *AST::makeExprBinop(INode *binop, INode *lhs, INode *rhs) {
	return new ExprBinOp{static_cast<BinOp *>(binop), static_cast<Expr *>(lhs), static_cast<Expr *>(rhs)};
}
INode *AST::makeExprAssign(INode *id, INode *val) {
	return new ExprAssign{static_cast<ExprId *>(id), static_cast<Expr *>(val)};
}
INode *AST::makeExprUnop(INode *unop, INode *expr) {
	return new ExprUnOp{static_cast<UnOp *>(unop), static_cast<Expr *>(expr)};
}
INode *AST::makeExprQmark() {
	return new ExprQmark{};
}

INode *AST::makeBinOpMul() {
	 return new BinOpMul{};
}
INode *AST::makeBinOpDiv() {
	 return new BinOpDiv{};
}
INode *AST::makeBinOpPlus() {
	 return new BinOpPlus{};
}
INode *AST::makeBinOpMinus() {
	 return new BinOpMinus{};
}
INode *AST::makeBinOpLess() {
	 return new BinOpLess{};
}
INode *AST::makeBinOpGrtr() {
	 return new BinOpGrtr{};
}
INode *AST::makeBinOpLessOrEq() {
	 return new BinOpLessOrEq{};
}
INode *AST::makeBinOpGrtrOrEq() {
	 return new BinOpGrtrOrEq{};
}
INode *AST::makeBinOpEqual() {
	 return new BinOpEqual{};
}
INode *AST::makeBinOpNotEqual() {
	 return new BinOpNotEqual{};
}

INode *AST::makeUnOpPlus() {
	 return new UnOpPlus{};
}
INode *AST::makeUnOpMinus() {
	 return new UnOpMinus{};
}
INode *AST::makeUnOpNot() {
	 return new UnOpNot{};
}

INode *AST::makeExprApply(INode *id, INode *ops) {
	return new ExprApply{static_cast<ExprId *>(id), static_cast<ExprList *>(ops)};
}
INode *AST::makeFunc(INode *scope, INode *declist) {
	return new Func{static_cast<Scope *>(scope), static_cast<Declist *>(declist)};
}
INode *AST::makeFunc(INode *scope, INode *declist, INode *id) {
	return new Func{static_cast<Scope *>(scope), static_cast<Declist *>(declist), static_cast<ExprId *>(id)};
}

INode *AST::makeDeclist(INode *declist, INode *id) {
	static_cast<Declist *>(declist)->push_front(static_cast<ExprId *>(id)->name);
	delete id;
	return declist;
}

INode *AST::makeDeclistTerm() {
	return new Declist{};
}

INode *AST::makeExprlist(INode *exprlist, INode *expr) {
	static_cast<ExprList *>(exprlist)->push_front(static_cast<Expr *>(expr));
	return exprlist;
}

INode *AST::makeExprlistTerm() {
	return new ExprList{};
}
