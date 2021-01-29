#pragma once
#include "inode.hh"
#include "exec.h"
#include <iostream>
#include <map>
#include <stdexcept>
#include <list>
#include <typeinfo>

namespace AST {

struct Func;

struct IValue {
	virtual int &get_int() = 0;
	virtual Func &get_Func() = 0;
	virtual IValue &operator = (const IValue &rhs) = 0;
	virtual IValue *clone() = 0;
	virtual ~IValue() {};
	static IValue *defaultValue();
};

using VarsT = std::map<std::string, IValue *>;

struct Node {
	Node *parent = nullptr;
	virtual ~Node() {};
};

struct IntValue : public IValue {
	private:
	int val_;
	public:
	int &get_int() override {
		return val_;
	}
	Func &get_Func() override {
		throw std::logic_error("Int as Func");
	}
	IntValue &operator = (const IValue &rhs) override {
		val_ = static_cast<const IntValue &>(rhs).val_;
		return *this;
	}
	IntValue *clone() override {
		return new IntValue(val_);
	}
	IntValue(int v) : val_(v)
	{}
};

inline IValue *IValue::defaultValue() {
	return new IntValue{0};
}

struct Expr : public Node, public IExecable{
	virtual IValue *eval() = 0;
	void exec() {
		IValue *res = nullptr;
		try {
			res = eval();
		} catch (std::logic_error& err)
		{
			std::cout << "Semantic error: " << err.what() << std::endl;
		}
		delete res;
	}
};

struct Block : public Expr {
};

struct BinOp : public Node, public INode {
	template <typename F>
	IntValue *as_int(F functor, Expr *lhs, Expr *rhs) {
		auto val_l = lhs->eval();
		auto val_r = rhs->eval();
		int res = functor(val_l->get_int(), val_r->get_int());
		delete val_l;
		delete val_r;
		return new IntValue{res};
	}
	virtual IValue *operator() (Expr *lhs, Expr *rhs) = 0;
};

struct UnOp : public Node, public INode {
	template <typename F>
	IntValue *as_int(F functor, Expr *rhs) {
		auto val = rhs->eval();
		int res = functor(val->get_int());
		delete val;
		return new IntValue{res};
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
		IValue *res = nullptr;
		if (head)
			res = head->eval();
		if (tail) {
			delete res;
			res = tail->eval();
		}
		return res;
	}
};

struct Scope : public Block {
	VarsT vars_;
	Blocks *blocks;
	Scope(Blocks *b) : blocks(b) {
		if (blocks)
			blocks->parent = this;
	}
	~Scope() {
		delete blocks;
		for (auto elem : vars_)
			delete elem.second;
	}
	IValue *eval() override {
		if (blocks)
			return blocks->eval();
		return IValue::defaultValue();
	}
	void assign(const std::string &name, IValue *new_val) {
		try {
			auto &dest = vars_.at(name);
			if (dest != new_val) {
				delete dest;
				dest = new_val;
			}
		}
		catch (std::out_of_range) {
			vars_[name] = new_val;
		}
	}
};

struct Declist : public INode, public std::list<std::string> {
};

struct ExprList : public INode, public std::list<Expr *> {
	~ExprList() {
		for (auto expr : *this)
			delete expr;
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
		auto val = expr->eval();
		std::cout << val->get_int() << std::endl;
		return val;
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
		IValue *res = IValue::defaultValue();
		IValue *cond;
		while ((cond = expr->eval())->get_int()) {
			delete cond;
			delete res;
			res = block->eval();
		}
		delete cond;
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
		auto cond = expr->eval();
		IValue *res;
		if (cond->get_int())
			res = true_block->eval();
		else if (false_block)
			res = false_block->eval();
		else
			res = IValue::defaultValue();
		delete cond;
		return res;
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
					return static_cast<Scope *>(node)->vars_.at(name)->clone();
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
		auto prev_vars = body_->vars_;
		if (ops->size() != decls_->size())
			throw std::logic_error("incorrect number of arguments in application");
		auto it = decls_->begin();
		for (auto expr : *ops)
			body_->vars_[*it++] = expr->eval();
		auto res = body_->eval();
		for (auto name : *decls_)
			delete body_->vars_.at(name);
		body_->vars_ = prev_vars;
		return res;
	}
};


struct FuncValue : public IValue {
	private:
	Func func_;
	public:
	int &get_int() override {
		throw std::logic_error("Func as Int");
	}
	Func &get_Func() override {
		return func_;
	}
	FuncValue &operator = (const IValue &rhs) override {
		func_ = static_cast<const FuncValue &>(rhs).func_;
		return *this;
	}
	FuncValue *clone() override {
		return new FuncValue(func_);
	}
	FuncValue(Func f) : func_(f)
	{}
};

inline IValue *ExprFunc::eval() {
	if (id) {
		auto node = parent;
		while (node->parent)
			node = node->parent;
		static_cast<Scope *>(node)->assign(id->name, new FuncValue(this));
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
					auto &dest = static_cast<Scope *>(node)->vars_.at(id->name);
					delete dest;
					dest = val;
				} catch (std::out_of_range) {
					if (!node->parent) {
						getScope()->vars_[id->name] = val;
					}
				}
		return  val->clone();
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
		for (auto expr : *ops)
			expr->parent = this;
	}
	~ExprApply() {
		delete id;
		delete ops;
	}
	IValue *eval() override {
		auto val = id->eval();
		auto res = val->get_Func()(ops);
		delete val;
		return res;
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
