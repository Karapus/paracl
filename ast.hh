#pragma once
#include "inode.hh"
#include "exec.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include <typeinfo>
#include <optional>
#include <functional>

namespace AST {

struct Func;

struct IValue {
	virtual operator int&() = 0;
	virtual operator Func&() = 0;
	virtual IValue *clone() const = 0;
	virtual ~IValue() {};
	static IValue *defaultValue();
};


struct Scope;
struct Node {
	Node *parent_ = nullptr;
	virtual ~Node() {};
};

struct IntValue : public IValue {
private:
	int val_;
public:
	operator int&() override {
		return val_;
	}
	operator Func&() override {
		throw std::logic_error("Int as Func");
	}
	IntValue *clone() const override {
		return new IntValue(val_);
	}
	IntValue(int val) : val_(val)
	{}
};

struct ExprFunc;
struct DeclList;
struct ExprList;
struct Value;

struct Func {
	Scope *body_;
	DeclList *decls_;
	Func(ExprFunc *func);
	Value operator () (ExprList *ops);
};

struct FuncValue : public IValue {
	private:
	Func func_;
	public:
	operator int&() override {
		throw std::logic_error("Func as Int");
	}
	operator Func&() override {
		return func_;
	}
	FuncValue *clone() const override {
		return new FuncValue(func_);
	}
	FuncValue(Func f) : func_(f)
	{}
};

struct Value {
	private:
	IValue *ptr_;
	public:
	Value() : ptr_(IValue::defaultValue()) {
	}
	Value(const Value &rhs) {
		ptr_ = rhs.ptr_->clone();
	}
	Value &operator = (const Value &rhs) {
		if (this != &rhs) {
			delete ptr_;
			ptr_ = rhs.ptr_->clone();
		}
		return *this;
	}
	Value(int val) : ptr_(new IntValue(val)) {
	}
	Value(Func val) : ptr_(new FuncValue(val)) {
	}
	operator int&() {
		return static_cast<int&>(*ptr_);
	}
	operator Func&() {
		return static_cast<Func&>(*ptr_);
	}
	operator bool() {
		return static_cast<int>(*ptr_);
	}
	~Value() {
		delete ptr_;
	}
};

using VarsT = std::map<std::string, Value>;

struct Expr : public Node, public IExecable{
	virtual Value eval() = 0;
	void exec();
	Scope *getScope();
};

struct BinOp : public Node, public INode {
	template <typename F>
	Value as_int(F functor, Expr *lhs, Expr *rhs) {
		auto val_l = lhs->eval();
		auto val_r = rhs->eval();
		val_l = functor(val_l, val_r);
		return val_l;
	}
	virtual Value operator() (Expr *lhs, Expr *rhs) = 0;
};

struct UnOp : public Node, public INode {
	template <typename F>
	Value as_int(F functor, Expr *rhs) {
		auto val = rhs->eval();
		val = functor(val);
		return val;
	}
	virtual Value operator() (Expr *rhs) = 0;
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
	Value eval() override;
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
	}
	Value eval() override; 
	bool assign(const std::string &name, Value new_val, bool forse_flag = true);
	std::optional<Value> resolve(const std::string &name) const;
};

struct StmExpr : public Expr {
	Expr *expr;
	StmExpr(Expr *e) : expr(e) {
		expr->parent_ = this;
	}
	~StmExpr() {
		delete expr;
	}
	Value eval() override;
};

struct StmPrint : public Expr {
	Expr *expr;
	StmPrint(Expr *e) : expr(e) {
		expr->parent_ = this;
	}
	~StmPrint() {
		delete expr;
	}
	Value eval() override;
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
	Value eval() override;
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
	Value eval() override;
};

struct ReturnExcept {
	Value val_;
	ReturnExcept(Value val) : val_(val) {
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
	Value eval() override;
};

struct ExprInt : public Expr {
	int val;
	ExprInt(int i) : val(i)
	{}
	Value eval() override;
};

struct ExprId : public Expr {
	std::string name;
	ExprId(std::string n) : name(n)
	{}
	Value eval() override;
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
	Value eval() override;
};

struct ExprQmark : public Expr {
	Value eval() override;
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
	Value eval() override;
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
	Value eval() override;
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
	Value eval() override;
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
	Value eval() override;
};

struct BinOpMul : public BinOp {
	Value operator() (Expr *lhs, Expr *rhs) override {
		return as_int(std::multiplies<int>{}, lhs, rhs);
	}
};

struct BinOpDiv : public BinOp {
	Value operator() (Expr *lhs, Expr *rhs) override {
		return as_int(std::divides<int>{}, lhs, rhs);
	}
};
struct BinOpPlus : public BinOp {
	Value operator() (Expr *lhs, Expr *rhs) override {
		return as_int(std::plus<int>{}, lhs, rhs);
	}
};
struct BinOpMinus : public BinOp {
	Value operator() (Expr *lhs, Expr *rhs) override {
		return as_int(std::minus<int>{}, lhs, rhs);
	}
};
struct BinOpLess : public BinOp {
	Value operator() (Expr *lhs, Expr *rhs) override {
		return as_int(std::less<int>{}, lhs, rhs);
	}
};
struct BinOpGrtr : public BinOp {
	Value operator() (Expr *lhs, Expr *rhs) override {
		return as_int(std::greater<int>{}, lhs, rhs);
	}
};
struct BinOpLessOrEq : public BinOp {
	Value operator() (Expr *lhs, Expr *rhs) override {
		return as_int(std::less_equal<int>{}, lhs, rhs);
	}
};
struct BinOpGrtrOrEq : public BinOp {
	Value operator() (Expr *lhs, Expr *rhs) override {
		return as_int(std::greater_equal<int>{}, lhs, rhs);
	}
};
struct BinOpEqual : public BinOp {
	Value operator() (Expr *lhs, Expr *rhs) override {
		return as_int(std::equal_to<int>{}, lhs, rhs);
	}
};
struct BinOpNotEqual : public BinOp {
	Value operator() (Expr *lhs, Expr *rhs) override {
		return as_int(std::not_equal_to<int>{}, lhs, rhs);
	}
};

struct UnOpPlus : public UnOp {
	Value operator() (Expr *rhs) override {
		return as_int([](int a) { return +a; }, rhs);
	}
};
struct UnOpMinus : public UnOp {
	Value operator() (Expr *rhs) override {
		return as_int(std::negate<int>{}, rhs);
	}
};
struct UnOpNot : public UnOp {
	Value operator() (Expr *rhs) override {
		return as_int(std::logical_not<int>{}, rhs);
	}
};
#if 0
Func::Func(ExprFunc *func) : body_(func->body), decls_(func->decls) 
{}
#endif
}
