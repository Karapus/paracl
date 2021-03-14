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
	return new Scope{static_cast<BlockList *>(blocks)};
}

INode *AST::makeBlockList(INode *blocks, INode *block) {
	return new BlockList(static_cast<BlockList *>(blocks), static_cast<Expr *>(block));
}

INode *AST::makeBlockListTerm() {
	return nullptr;
}

INode *AST::makeStmExpr(INode *expr) {
	return expr;
}

INode *AST::makeStmPrint(INode *expr) {
	return new StmPrint{static_cast<Expr *>(expr)};
}

INode *AST::makeStmWhile(INode *expr, INode *block) {
	return new StmWhile{static_cast<Expr *>(expr), static_cast<Expr *>(block)};
}

INode *AST::makeStmIf(INode *expr, INode *true_block, INode *false_block) {
	return new StmIf{static_cast<Expr *>(expr), static_cast<Expr *>(true_block), static_cast<Expr *>(false_block)};
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

INode *AST::makeExprFunc(INode *scope, INode *declist) {
	return new ExprFunc{static_cast<Scope *>(scope), static_cast<DeclList *>(declist)};
}

INode *AST::makeExprFunc(INode *scope, INode *declist, INode *id) {
	return new ExprFunc{static_cast<Scope *>(scope), static_cast<DeclList *>(declist), static_cast<ExprId *>(id)};
}

INode *AST::makeDeclList(INode *declist, INode *id) {
	static_cast<DeclList *>(declist)->push_back(static_cast<ExprId *>(id)->name_);
	delete id;
	return declist;
}

INode *AST::makeDeclListTerm() {
	return new DeclList{};
}

INode *AST::makeExprList(INode *exprlist, INode *expr) {
	return new ExprList{static_cast<ExprList *>(exprlist), static_cast<Expr *>(expr)};
}

INode *AST::makeExprListTerm() {
	return nullptr;
}

INode *AST::makeStmReturn(INode *expr) {
	return new StmReturn{static_cast<Expr *>(expr)};
}
