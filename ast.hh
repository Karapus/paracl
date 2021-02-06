#pragma once
#include "inode.hh"
#include "exec.h"
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include <typeinfo>

namespace AST {

struct Func;

struct IValue {
	virtual int &get_int() = 0;
	virtual Func &get_Func() = 0;
	virtual IValue &operator = (const IValue &rhs) = 0;
	virtual IValue *clone() const = 0;
	virtual ~IValue() {};
	static IValue *defaultValue();
};

using VarsT = std::map<std::string, IValue *>;

struct Scope;
struct Node {
	Node *parent_ = nullptr;
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
	IntValue *clone() const override {
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
	Scope *getScope();
};

struct BinOp : public Node, public INode {
	template <typename F>
	IValue *as_int(F functor, Expr *lhs, Expr *rhs) {
		auto val_l = lhs->eval();
		auto val_r = rhs->eval();
		val_l->get_int() = functor(val_l->get_int(), val_r->get_int());
		delete val_r;
		return val_l;
	}
	virtual IValue *operator() (Expr *lhs, Expr *rhs) = 0;
};

struct UnOp : public Node, public INode {
	template <typename F>
	IValue *as_int(F functor, Expr *rhs) {
		auto val = rhs->eval();
		val->get_int() = functor(val->get_int());
		return val;
	}
	virtual IValue *operator() (Expr *rhs) = 0;
};

struct BlockList : public Expr {
	private:
	std::vector<Expr *> cner_;
	public:
	~BlockList() {
		for (auto expr : cner_)
			delete expr;
	}
	void push_back(Expr *expr) {
		cner_.push_back(expr);
		expr->parent_ = this;
	}
	IValue *eval() override {
		IValue *res = IValue::defaultValue();
		for (auto expr : cner_) {
			delete res;
			res = expr->eval();
		}
		return res;
	}
};

struct DeclList : public INode {
	private:
	std::vector<std::string> cner_;
	public:
	auto begin() {
		return cner_.begin();
	}
	auto end() {
		return cner_.end();
	}
	auto size() const {
		return cner_.size();
	}
	auto push_back(const std::string &x) {
		return cner_.push_back(x);
	}
};

struct ExprList : public INode {
	private:
	std::vector<Expr *> cner_;
	public:
	~ExprList() {
		for (auto expr : cner_)
			delete expr;
	}
	auto begin() {
		return cner_.begin();
	}
	auto end() {
		return cner_.end();
	}
	auto size() const {
		return cner_.size();
	}
	auto push_back(Expr *expr) {
		return cner_.push_back(expr);
	}
};

struct Scope : public Expr {
	VarsT vars_;
	BlockList *blocks;
	Scope(BlockList *b) : blocks(b) {
		if (blocks)
			blocks->parent_ = this;
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
	bool assign(const std::string &name, IValue *new_val, bool forse_flag = true) {
		auto dest_it = vars_.find(name);
		if (dest_it != vars_.end()) {
			auto &dest = dest_it->second;
			if (dest != new_val) {
				delete dest;
				dest = new_val;
			}
			return true;
		}
		else if (forse_flag) {
			vars_[name] = new_val;
		}
		return forse_flag;
	}
	IValue *resolve(const std::string &name) const {
		auto it = vars_.find(name);
		return it != vars_.cend() ? it->second->clone() : nullptr; 
	}
};

inline Scope *Expr::getScope() {
	auto node = parent_;
	while (node && (typeid(*node) != typeid(Scope)))
		node = node->parent_;
	return static_cast<Scope *>(node);
}

struct StmExpr : public Expr {
	Expr *expr;
	StmExpr(Expr *e) : expr(e) {
		expr->parent_ = this;
	}
	~StmExpr() {
		delete expr;
	}
	IValue *eval() override {
		return expr->eval();
	}
};

struct StmPrint : public Expr {
	Expr *expr;
	StmPrint(Expr *e) : expr(e) {
		expr->parent_ = this;
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

struct StmWhile : public Expr {
	Expr *expr;
	Expr *block;
	StmWhile(Expr *e, Expr *b) : expr(e), block(b) {
		expr->parent_ = this;
		block->parent_ = this;
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

struct StmIf : public Expr {
	Expr *expr;
	Expr *true_block;
	Expr *false_block;
	StmIf(Expr *e, Expr *tb, Expr *fb) : expr(e), true_block(tb), false_block(fb) {
		expr->parent_ = this;
		true_block->parent_ = this;
		if (false_block)
			false_block->parent_ = this;
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

struct ReturnExcept {
	IValue *val_;
	ReturnExcept(IValue *val) : val_(val) {
	}
};

struct StmReturn : public Expr {
	Expr *expr_;
	StmReturn(Expr *expr) : expr_(expr) {
		expr_->parent_ = this;
	}
	~StmReturn() {
		delete expr_;
	}
	IValue *eval() {
		throw ReturnExcept(expr_->eval());
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
		for (auto scope = getScope(); scope; scope = scope->getScope()) {
			auto val = scope->resolve(name);
			if (val)
				return val;
		}
		return IValue::defaultValue();
	}
};

struct ExprFunc : public Expr {
	Scope *body;
	DeclList *decls;
	ExprId *id;
	ExprFunc(Scope *b, DeclList *d, ExprId *i = nullptr) : body(b), decls(d), id(i) {
		body->parent_ = this;
			}
	~ExprFunc() {
		delete body;
		delete decls;
	}
	IValue *eval() override;
};


struct Func {
	Scope *body_;
	DeclList *decls_;
	Func(ExprFunc *func) : body_(func->body), decls_(func->decls) 
	{}
	IValue *operator () (ExprList *ops) {
		auto prev_vars = body_->vars_;
		if (ops->size() != decls_->size())
			throw std::logic_error("incorrect number of arguments in application");
		auto it = decls_->begin();
		for (auto expr : *ops)
			body_->vars_[*it++] = expr->eval();
		IValue *res;
		try {
			res = body_->eval();
		} catch (ReturnExcept &ret) {
			res = ret.val_;
		}
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
	FuncValue *clone() const override {
		return new FuncValue(func_);
	}
	FuncValue(Func f) : func_(f)
	{}
};

inline IValue *ExprFunc::eval() {
	if (id) {
		auto node = parent_;
		while (node->parent_)
			node = node->parent_;
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
		id->parent_ = this;
		expr->parent_ = this;
	}
	~ExprAssign() {
		delete id;
		delete expr;
	}
	IValue *eval() override {
		IValue *val = expr->eval();
		const auto &name = id->name;
		bool flag;
		for (auto scope = getScope(); scope && (flag = !scope->assign(name, val, false)); scope = scope->getScope())
			;
		if (flag) getScope()->assign(name, val);
		return  val->clone();
	}
};

struct ExprApply : public Expr {
	ExprId *id;
	ExprList *ops;
	ExprApply(ExprId *i, ExprList *o) : id(i), ops(o) {
		id->parent_ = this;
		for (auto expr : *ops)
			expr->parent_ = this;
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
		op->parent_ = this;
		lhs->parent_ = this;
		rhs->parent_ = this;
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
		op->parent_ = this;
		rhs->parent_ = this;
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
