#pragma once
#include "inode.hh"
#include "exec.h"
#include <cstdlib>
#include <iostream>
#include <map>
#include <stdexcept>
#include <variant>
#include <list>
#include <algorithm>
#include <typeinfo>

namespace AST {

struct Func;

struct IValue {
	virtual int *get_int() = 0;
	virtual Func *get_Func() = 0;
	virtual ~IValue() {};
	static IValue *defaultValue();
};

using VarsT = std::map<std::string, IValue *>;

struct Node {
	Node *parent = nullptr;
	virtual ~Node() {};
};

struct IntValue : public IValue {
	int val;
	int *get_int() override {
		return &val;
	}
	Func *get_Func() override {
		throw std::logic_error("Int as Func");
	}
	IntValue(int v) : val(v)
	
{}
};

inline IValue *IValue::defaultValue() {
	return new IntValue{0};
}

struct Expr : public Node, public IExecable{
	virtual IValue *eval() = 0;
	void exec() {
		eval();
	}
};

struct Block : public Expr {
};

struct BinOp : public Node, public INode {
	template <typename F>
	IntValue *as_int(F functor, Expr *lhs, Expr *rhs) {
		return new IntValue{functor(*(lhs->eval()->get_int()), *(rhs->eval()->get_int()))};
	}
	virtual IValue *operator() (Expr *lhs, Expr *rhs) = 0;
};

struct UnOp : public Node, public INode {
	template <typename F>
	IntValue *as_int(F functor, Expr *rhs) {
		return new IntValue{functor(*(rhs->eval()->get_int()))};
	}
	virtual IValue *operator() (Expr *rhs) = 0;
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
	IValue *eval() override {
		IValue *res;
		if (head)
			res = head->eval();
		if (tail)
			res = tail->eval();
		return res;
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
	IValue *eval() override {
		if (blocks)
			return blocks->eval();
		return IValue::defaultValue();
	}
};

struct Declist : public INode, public std::list<std::string> {
};

struct ExprList : public INode, public std::list<Expr *> {
};

struct StmExpr : public Stm {
	Expr *expr;
	StmExpr(Expr *e) : expr(e) {
		expr->parent = this;
	}
	~StmExpr() {
		delete expr;
	}
	IValue *eval() override {
		return expr->eval();
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
	IValue *eval() override {
		std::cout << *expr->eval()->get_int() << std::endl;
		return IValue::defaultValue();
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
	IValue *eval() override {
		IValue *res;
		while (*(expr->eval()->get_int()))
			res = block->eval();
		return res;
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
	IValue *eval() override {
		if (*(expr->eval()->get_int()))
			return true_block->eval();
		else if (false_block)
			return false_block->eval();
		return IValue::defaultValue();
	}
};

struct ExprInt : public Expr {
	int val;
	ExprInt(int i) : val(i)
	{}
	IValue *eval() override {
		return new IntValue{val};
	}
};

struct ExprId : public Expr {
	std::string name;
	ExprId(std::string n) : name(n)
	{}
	IValue *eval() override {
		for (Node *node = parent; node; node = node->parent)
			if (typeid(*node) == typeid(Scope)) {
				try {
					return static_cast<Scope *>(node)->vars.at(name);
				} catch (std::out_of_range) {
				}
			}
		return IValue::defaultValue();
	}
};

struct ExprFunc : public Expr {
	Scope *body;
	Declist *decls;
	ExprId *id;
	ExprFunc(Scope *b, Declist *d, ExprId *i = nullptr) : body(b), decls(d), id(i) {
		body->parent = this;
			}
	~ExprFunc() {
		delete body;
		delete decls;
	}
	IValue *eval() override;
};

struct Func {
	Scope *body_;
	Declist *decls_;
	Func(ExprFunc *func) : body_(func->body), decls_(func->decls) 
	{}
	IValue *operator () (ExprList *ops) {
		auto prev_vars = body_->vars;
		if (ops->size() != decls_->size())
			throw std::logic_error("incorrect number of arguments in application");
		std::for_each(ops->begin(), ops->end(), [=, it = decls_->begin() ](Expr *expr) mutable { body_->vars[*it++] = expr->eval();});
		auto res = body_->eval();
		body_->vars = prev_vars;
		return res;
	}
};


struct FuncValue : public IValue {
	Func func;
	int *get_int() override {
		throw std::logic_error("Func as Int");
	}
	Func *get_Func() override {
		return &func;
	}
	FuncValue(Func f) : func(f)
	{}
};

inline IValue *ExprFunc::eval() {
	if (id) {
		auto node = parent;
		while (node->parent)
			node = node->parent;
		static_cast<Scope *>(node)->vars[id->name] = new FuncValue(this);
		delete id;
		id = nullptr;
	}
	return new FuncValue(this);
}

struct ExprQmark : public Expr {
	IValue *eval() override {
		int val;
		std::cin >> val;
		return new IntValue{val};
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
	IValue *eval() override {
		IValue *val = expr->eval();
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

struct ExprApply : public Expr {
	ExprId *id;
	ExprList *ops;
	ExprApply(ExprId *i, ExprList *o) : id(i), ops(o) {
		id->parent = this;
		std::for_each(ops->begin(), ops->end(), [this] (auto &op) { op->parent = this; });
	}
	~ExprApply() {
		delete id;
		delete ops;
	}
	IValue *eval() override {
		return (*id->eval()->get_Func())(ops);
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
	IValue *eval() override {
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
	IValue *eval() override {
		return (*op)(rhs);
	}
};

struct BinOpMul : public BinOp {
	IValue *operator() (Expr *lhs, Expr *rhs) override {
		return as_int([](int a, int b){ return a * b; }, lhs, rhs);
	}
};

struct BinOpDiv : public BinOp {
	IValue *operator() (Expr *lhs, Expr *rhs) override {
		return as_int([](int a, int b) { return a / b; }, lhs, rhs);
	}
};
struct BinOpPlus : public BinOp {
	IValue *operator() (Expr *lhs, Expr *rhs) override {
		return as_int([](int a, int b) { return a + b; }, lhs, rhs);
	}
};
struct BinOpMinus : public BinOp {
	IValue *operator() (Expr *lhs, Expr *rhs) override {
		return as_int([](int a, int b) { return a - b; }, lhs, rhs);
	}
};
struct BinOpLess : public BinOp {
	IValue *operator() (Expr *lhs, Expr *rhs) override {
		return as_int([](int a, int b) { return a < b; }, lhs, rhs);
	}
};
struct BinOpGrtr : public BinOp {
	IValue *operator() (Expr *lhs, Expr *rhs) override {
		return as_int([](int a, int b) { return a > b; }, lhs, rhs);
	}
};
struct BinOpLessOrEq : public BinOp {
	IValue *operator() (Expr *lhs, Expr *rhs) override {
		return as_int([](int a, int b) { return a <= b; }, lhs, rhs);
	}
};
struct BinOpGrtrOrEq : public BinOp {
	IValue *operator() (Expr *lhs, Expr *rhs) override {
		return as_int([](int a, int b) { return a >= b; }, lhs, rhs);
	}
};
struct BinOpEqual : public BinOp {
	IValue *operator() (Expr *lhs, Expr *rhs) override {
		return as_int([](int a, int b) { return a == b; }, lhs, rhs);
	}
};
struct BinOpNotEqual : public BinOp {
	IValue *operator() (Expr *lhs, Expr *rhs) override {
		return as_int([](int a, int b) { return a != b; }, lhs, rhs);
	}
};

struct UnOpPlus : public UnOp {
	IValue *operator() (Expr *rhs) override {
		return as_int([](int a) { return +a; }, rhs);
	}
};
struct UnOpMinus : public UnOp {
	IValue *operator() (Expr *rhs) override {
		return as_int([](int a) { return -a; }, rhs);
	}
};
struct UnOpNot : public UnOp {
	IValue *operator() (Expr *rhs) override {
		return as_int([](int a) { return !a; }, rhs);
	}
};
}
