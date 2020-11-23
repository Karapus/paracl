#pragma once
#include "inode.h"
#include "exec.h"
#include <iostream>
#include <map>
#include <typeinfo>

namespace AST {
using VarsT = std::map<std::string, int>;

class Node {
public:
	Node *parent = nullptr;
	virtual ~Node() {};
};

class Block : public Node, public IExecable {
};

class Expr : public Node, public INode {
public:
	virtual int eval() = 0;
};

class BinOp : public Node, public INode {
public:
	virtual int operator() (Expr *lhs, Expr *rhs) = 0;
};

class UnOp : public Node, public INode {
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

class Scope : public Block {
public:
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

class StmExpr : public Stm {
public:
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

class StmPrint : public Stm {
public:
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

class StmWhile : public Stm {
public:
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

class StmIf : public Stm {
public:
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
						static_cast<Scope *>(getScope())->vars[id->name] = val;
					}
				}
		return val;
	}
	Node *getScope() {
		auto scope = parent;
		for (; typeid(*scope) != typeid(Scope); scope = scope->parent)
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

class ExprUnOp : public Expr {
public:
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
	int operator() (Expr *rhs) override {
		return +rhs->eval();
	}
};
class UnOpMinus : public UnOp {
public:
	int operator() (Expr *rhs) override {
		return -rhs->eval();
	}
};
class UnOpNot : public UnOp {
public:
	int operator() (Expr *rhs) override {
		return !rhs->eval();
	}
};
}
