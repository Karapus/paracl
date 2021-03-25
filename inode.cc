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

INode *AST::makeEmpty() {
	return new Empty{};
}

INode *AST::makeWhile(INode *expr, INode *block) {
	return new While{static_cast<Expr *>(expr), static_cast<Expr *>(block)};
}

INode *AST::makeIf(INode *expr, INode *true_block, INode *false_block) {
	return new If{static_cast<Expr *>(expr), static_cast<Expr *>(true_block), static_cast<Expr *>(false_block)};
}

INode *AST::makeSeq(INode *fst, INode *snd) {
	return new Seq{static_cast<Expr *>(fst), static_cast<Expr *>(snd)};
}

INode *AST::makeExprAssign(INode *id, INode *val) {
	return new ExprAssign{static_cast<ExprId *>(id), static_cast<Expr *>(val)};
}

INode *AST::makeExprQmark() {
	return new ExprQmark{};
}

INode *AST::makeExprApply(INode *id, INode *ops) {
	return new ExprApply{static_cast<ExprId *>(id), static_cast<ExprList *>(ops)};
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

INode *AST::makeReturn(INode *expr) {
	return new Return{static_cast<Expr *>(expr)};
}
