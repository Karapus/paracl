#pragma once
#include <iostream>
#include <map>
#include <typeinfo>
#include <cassert>
#include "inode.h"

namespace AST {
using VarsT = std::map<std::string, int>;

class INode {
	public:
	INode *parent_ = nullptr;
public:
	virtual ~INode()
	{}
	void set_parent(INode *new_p) {
		parent_ = new_p;
	}
};

class Block : public INode {
public:
	virtual void exec() = 0;
};

class Expr : public INode {
public:
	virtual int eval() = 0;
};

class BinOp : public INode {
public:
	virtual int operator() (Expr *lhs, Expr *rhs) = 0;
};

class UnOp : public INode {
public:
	virtual int operator() (Expr *rhs) = 0;
};

class Stm : public Block {
};

class Blocks : public Block {
public:
	Block *head;
	Blocks *tail;
	Blocks(Block *h, Blocks *t) : head(h), tail(t)
	{
		if (head)
			head->set_parent(this);
		if (tail)
			tail->set_parent(this);
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

class Scope : public Block {
public:
	VarsT vars;
	Blocks *blocks;
	Scope(Blocks *b) : blocks(b) {
		if (blocks)
			blocks->set_parent(this);
	}
	~Scope() {
		delete blocks;
	}
	void exec() override {
		if (blocks)
			blocks->exec();
	}
};

class StmExpr : public Stm {
public:
	Expr *expr;
	StmExpr(Expr *e) : expr(e) {
		expr->set_parent(this);
	}
	~StmExpr() {
		delete expr;
	}
	void exec() override {
		expr->eval();
	}
};

class StmPrint : public Stm {
public:
	Expr *expr;
	StmPrint(Expr *e) : expr(e) {
		expr->set_parent(this);
	}
	~StmPrint() {
		delete expr;
	}
	void exec() {
		std::cout << expr->eval() << std::endl;
	}
};

class StmWhile : public Stm {
public:
	Expr *expr;
	Block *block;
	StmWhile(Expr *e, Block *b) : expr(e), block(b) {
		expr->set_parent(this);
		block->set_parent(this);
	}
	~StmWhile() {
		delete expr;
		delete block;
	}
	void exec() {
		while (expr->eval())
			block->exec();
	}
};

class StmIf : public Stm {
public:
	Expr *expr;
	Block *true_block;
	Block *false_block;
	StmIf(Expr *e, Block *tb, Block *fb) : expr(e), true_block(tb), false_block(fb) {
		expr->set_parent(this);
		true_block->set_parent(this);
		if (false_block)
			false_block->set_parent(this);
	}
	~StmIf() {
		delete expr;
		delete true_block;
		delete false_block;
	}
	void exec() {
		if (expr->eval())
			true_block->exec();
		else if (false_block)
			false_block->exec();
	}
};

class ExprInt : public Expr {
public:
	int val;
	ExprInt(int i) : val(i)
	{}
	int eval() override {
		return val;
	}
};

class ExprId : public Expr {
public:
	std::string name;
	ExprId(std::string n) : name(n)
	{}
	int eval() override {
		for (INode *node = parent_; node; node = node->parent_)
			if (typeid(*node) == typeid(Scope)) {
				try {
					return static_cast<Scope *>(node)->vars.at(name);
				} catch (std::out_of_range) {
				}
			}
		return 0;
	}
	//rvlaue / lvalue difference
};

class ExprQmark : public Expr {
public:
	int eval() override {
		int val;
		std::cin >> val;
		return val;
	}
};
class ExprAssign : public Expr {
public:
	ExprId *id;
	Expr *expr;
	ExprAssign(ExprId *i, Expr *e) : id(i), expr(e) {
		id->set_parent(this);
		expr->set_parent(this);
	}
	~ExprAssign() {
		delete id;
		delete expr;
	}
	int eval() override {
		int val = expr->eval();
		for (INode *node = parent_; node; node = node->parent_)
			if (typeid(*node) == typeid(Scope))
				try {
					static_cast<Scope *>(node)->vars.at(id->name) = val;
				} catch (std::out_of_range) {
					if (!node->parent_) {
						static_cast<Scope *>(getScope())->vars[id->name] = val;
					}
				}
		return val;
	}
	INode *getScope() {
		auto scope = parent_;
		for (; typeid(*scope) != typeid(Scope); scope = scope->parent_)
			{}
		return scope; 
	}
};

class ExprBinOp : public Expr {
public:
	BinOp *op;
	Expr *lhs;
	Expr *rhs;
	ExprBinOp(BinOp *o, Expr *l, Expr *r) : op(o), lhs(l), rhs(r) {
		op->set_parent(this);
		lhs->set_parent(this);
		rhs->set_parent(this);
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

class ExprUnOp : public Expr {
public:
	UnOp *op;
	Expr *rhs;
	ExprUnOp(UnOp *o, Expr *r) : op(o), rhs(r) {
		op->set_parent(this);
		rhs->set_parent(this);
	}
	~ExprUnOp() {
		delete op;
		delete rhs;
	}
	int eval() override {
		return (*op)(rhs);
	}
};

class BinOpMul : public BinOp {
public:
	int operator() (Expr *lhs, Expr *rhs) override {
		return lhs->eval() * rhs->eval();
	}
};

class BinOpDiv : public BinOp {
public:
	int operator() (Expr *lhs, Expr *rhs) override {
		return lhs->eval() / rhs->eval();
	}
};
class BinOpPlus : public BinOp {
public:
	int operator() (Expr *lhs, Expr *rhs) override {
		return lhs->eval() + rhs->eval();
	}
};
class BinOpMinus : public BinOp {
public:
	int operator() (Expr *lhs, Expr *rhs) override {
		return lhs->eval() - rhs->eval();
	}
};
class BinOpLess : public BinOp {
public:
	int operator() (Expr *lhs, Expr *rhs) override {
		return lhs->eval() < rhs->eval();
	}
};
class BinOpGrtr : public BinOp {
public:
	int operator() (Expr *lhs, Expr *rhs) override {
		return lhs->eval() > rhs->eval();
	}
};
class BinOpLessOrEq : public BinOp {
public:
	int operator() (Expr *lhs, Expr *rhs) override {
		return lhs->eval() <= rhs->eval();
	}
};
class BinOpGrtrOrEq : public BinOp {
public:
	int operator() (Expr *lhs, Expr *rhs) override {
		return lhs->eval() >= rhs->eval();
	}
};
class BinOpEqual : public BinOp {
public:
	int operator() (Expr *lhs, Expr *rhs) override {
		return lhs->eval() == rhs->eval();
	}
};
class BinOpNotEqual : public BinOp {
public:
	int operator() (Expr *lhs, Expr *rhs) override {
		return lhs->eval() != rhs->eval();
	}
};

class UnOpPlus : public UnOp {
public:
	int operator() (Expr *rhs) {
		return +rhs->eval();
	}
};
class UnOpMinus : public UnOp {
public:
	int operator() (Expr *rhs) {
		return -rhs->eval();
	}
};
class UnOpNot : public UnOp {
public:
	int operator() (Expr *rhs) {
		return !rhs->eval();
	}
};
}
