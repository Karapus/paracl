#pragma once
#include "inode.h"
#include "exec.h"
#include <iostream>
#include <map>
#include <typeinfo>

namespace AST {
using VarsT = std::map<std::string, int>;

struct Node {
	Node *parent = nullptr;
	virtual ~Node() {};
};

struct Block : public Node, public IExecable {
};

struct Expr : public Node, public INode {
	virtual int eval() = 0;
};

struct BinOp : public Node, public INode {
	virtual int operator() (Expr *lhs, Expr *rhs) = 0;
};

struct UnOp : public Node, public INode {
	virtual int operator() (Expr *rhs) = 0;
};

struct Stm : public Block {
};

struct Blocks : public Block {
	Block *head;
	Blocks *tail;
	Blocks(Block *h, Blocks *t) : head(h), tail(t)
	{
		if (head) 
			head->parent = this;
		if (tail)
			tail->parent = this;
	}
	~Blocks() {
		delete head;
		delete tail;
	}
	void exec() override {
		if (head)
			head->exec();
		if (tail)
			tail->exec();
	}
};

struct Scope : public Block {
	VarsT vars;
	Blocks *blocks;
	Scope(Blocks *b) : blocks(b) {
		if (blocks)
			blocks->parent = this;
	}
	~Scope() {
		delete blocks;
	}
	void exec() override {
		if (blocks)
			blocks->exec();
	}
};

struct StmExpr : public Stm {
	Expr *expr;
	StmExpr(Expr *e) : expr(e) {
		expr->parent = this;
	}
	~StmExpr() {
		delete expr;
	}
	void exec() override {
		expr->eval();
	}
};

struct StmPrint : public Stm {
	Expr *expr;
	StmPrint(Expr *e) : expr(e) {
		expr->parent = this;
	}
	~StmPrint() {
		delete expr;
	}
	void exec() override {
		std::cout << expr->eval() << std::endl;
	}
};

struct StmWhile : public Stm {
	Expr *expr;
	Block *block;
	StmWhile(Expr *e, Block *b) : expr(e), block(b) {
		expr->parent = this;
		block->parent = this;
	}
	~StmWhile() {
		delete expr;
		delete block;
	}
	void exec() override {
		while (expr->eval())
			block->exec();
	}
};

struct StmIf : public Stm {
	Expr *expr;
	Block *true_block;
	Block *false_block;
	StmIf(Expr *e, Block *tb, Block *fb) : expr(e), true_block(tb), false_block(fb) {
		expr->parent = this;
		true_block->parent = this;
		if (false_block)
			false_block->parent = this;
	}
	~StmIf() {
		delete expr;
		delete true_block;
		delete false_block;
	}
	void exec() override {
		if (expr->eval())
			true_block->exec();
		else if (false_block)
			false_block->exec();
	}
};

struct ExprInt : public Expr {
	int val;
	ExprInt(int i) : val(i)
	{}
	int eval() override {
		return val;
	}
};

struct ExprId : public Expr {
	std::string name;
	ExprId(std::string n) : name(n)
	{}
	int eval() override {
		for (Node *node = parent; node; node = node->parent)
			if (typeid(*node) == typeid(Scope)) {
				try {
					return static_cast<Scope *>(node)->vars.at(name);
				} catch (std::out_of_range) {
				}
			}
		return 0;
	}
};

struct ExprQmark : public Expr {
	int eval() override {
		int val;
		std::cin >> val;
		return val;
	}
};
struct ExprAssign : public Expr {
	ExprId *id;
	Expr *expr;
	ExprAssign(ExprId *i, Expr *e) : id(i), expr(e) {
		id->parent = this;
		expr->parent = this;
	}
	~ExprAssign() {
		delete id;
		delete expr;
	}
	int eval() override {
		int val = expr->eval();
		for (Node *node = parent; node; node = node->parent)
			if (typeid(*node) == typeid(Scope))
				try {
					static_cast<Scope *>(node)->vars.at(id->name) = val;
				} catch (std::out_of_range) {
					if (!node->parent) {
						getScope()->vars[id->name] = val;
					}
				}
		return val;
	}
	Scope *getScope() {
		auto scope = parent;
		for (; typeid(*scope) != typeid(Scope); scope = scope->parent)
			{}
		return static_cast<Scope *>(scope);
	}
};

struct ExprBinOp : public Expr {
	BinOp *op;
	Expr *lhs;
	Expr *rhs;
	ExprBinOp(BinOp *o, Expr *l, Expr *r) : op(o), lhs(l), rhs(r) {
		op->parent = this;
		lhs->parent = this;
		rhs->parent = this;
	}
	~ExprBinOp() {
		delete op;
		delete lhs;
		delete rhs;
	}
	int eval() override {
		return (*op)(lhs, rhs);
	}
};

struct ExprUnOp : public Expr {
	UnOp *op;
	Expr *rhs;
	ExprUnOp(UnOp *o, Expr *r) : op(o), rhs(r) {
		op->parent = this;
		rhs->parent = this;
	}
	~ExprUnOp() {
		delete op;
		delete rhs;
	}
	int eval() override {
		return (*op)(rhs);
	}
};

struct BinOpMul : public BinOp {
	int operator() (Expr *lhs, Expr *rhs) override {
		return lhs->eval() * rhs->eval();
	}
};

struct BinOpDiv : public BinOp {
	int operator() (Expr *lhs, Expr *rhs) override {
		return lhs->eval() / rhs->eval();
	}
};
struct BinOpPlus : public BinOp {
	int operator() (Expr *lhs, Expr *rhs) override {
		return lhs->eval() + rhs->eval();
	}
};
struct BinOpMinus : public BinOp {
	int operator() (Expr *lhs, Expr *rhs) override {
		return lhs->eval() - rhs->eval();
	}
};
struct BinOpLess : public BinOp {
	int operator() (Expr *lhs, Expr *rhs) override {
		return lhs->eval() < rhs->eval();
	}
};
struct BinOpGrtr : public BinOp {
	int operator() (Expr *lhs, Expr *rhs) override {
		return lhs->eval() > rhs->eval();
	}
};
struct BinOpLessOrEq : public BinOp {
	int operator() (Expr *lhs, Expr *rhs) override {
		return lhs->eval() <= rhs->eval();
	}
};
struct BinOpGrtrOrEq : public BinOp {
	int operator() (Expr *lhs, Expr *rhs) override {
		return lhs->eval() >= rhs->eval();
	}
};
struct BinOpEqual : public BinOp {
	int operator() (Expr *lhs, Expr *rhs) override {
		return lhs->eval() == rhs->eval();
	}
};
struct BinOpNotEqual : public BinOp {
	int operator() (Expr *lhs, Expr *rhs) override {
		return lhs->eval() != rhs->eval();
	}
};

struct UnOpPlus : public UnOp {
	int operator() (Expr *rhs) override {
		return +rhs->eval();
	}
};
struct UnOpMinus : public UnOp {
	int operator() (Expr *rhs) override {
		return -rhs->eval();
	}
};
struct UnOpNot : public UnOp {
	int operator() (Expr *rhs) override {
		return !rhs->eval();
	}
};
}
