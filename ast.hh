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
#include <memory>

namespace AST {

struct Func;

struct IValue {
	virtual operator int&() = 0;
	virtual operator Func&() = 0;
	virtual IValue *clone() const = 0;
	virtual ~IValue() = default;
};


struct Scope;
struct Node {
	Node *parent_ = nullptr;
	virtual ~Node() = default;

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

struct DefaultValue : public IValue {
	operator int&() override {
		throw std::logic_error("Undefined value as Int");
	}
	operator Func&() override {
		throw std::logic_error("Undefined value as Func");
	}
	DefaultValue *clone() const override {
		return new DefaultValue{};
	}
};

struct Value {
	private:
	IValue *ptr_;
	public:
	Value() : ptr_(new DefaultValue{}) {
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
	void exec() override;
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
	private:
	VarsT vars_;
	BlockList *blocks;
	public:
	Scope(BlockList *b) : blocks(b) {
		blocks->parent_ = this;
	}
	~Scope() {
		delete blocks;
	}
	VarsT getVars() const {
		return vars_;
	}
	void setVars(const VarsT vars) {
		vars_ = vars;
	}
	Value eval() override; 
	void assign(const std::string &name, Value new_val);
	bool insert(const std::string &name, Value new_val);
	std::optional<Value> resolve(const std::string &name) const;
};

struct StmExpr : public Expr {
private:
	std::unique_ptr<Expr> expr_;
public:
	StmExpr(Expr *e) : expr_(e) {
		expr_->parent_ = this;
	}
	Value eval() override;
};

struct StmPrint : public Expr {
private:
	std::unique_ptr<Expr> expr_;
public:
	StmPrint(Expr *e) : expr_(e) {
		expr_->parent_ = this;
	}
	Value eval() override;
};

struct StmWhile : public Expr {
private:
	std::unique_ptr<Expr> expr_;
	std::unique_ptr<Expr> block_;
public:
	StmWhile(Expr *e, Expr *b) : expr_(e), block_(b) {
		expr_->parent_ = this;
		block_->parent_ = this;
	}
	Value eval() override;
};

struct StmIf : public Expr {
private:
	std::unique_ptr<Expr> expr_;
	std::unique_ptr<Expr> true_block_;
	std::unique_ptr<Expr> false_block_;
public:
	StmIf(Expr *e, Expr *tb, Expr *fb) : expr_(e), true_block_(tb), false_block_(fb) {
		expr_->parent_ = this;
		true_block_->parent_ = this;
		if (false_block_)
			false_block_->parent_ = this;
	}
	Value eval() override;
};

struct ReturnExcept {
	Value val_;
	ReturnExcept(Value val) : val_(val) {
	}
};

struct StmReturn : public Expr {
private:
	std::unique_ptr<Expr> expr_;
public:
	StmReturn(Expr *expr) : expr_(expr) {
		expr_->parent_ = this;
	}
	Value eval() override;
};

struct ExprInt : public Expr {
private:
	int val_;
public:
	ExprInt(int i) : val_(i)
	{}
	Value eval() override;
};

struct ExprId : public Expr {
	std::string name_;
	ExprId(std::string n) : name_(n)
	{}
	Value eval() override;
};

struct ExprFunc : public Expr {
	Scope *body_;
	DeclList *decls_;
	ExprId *id_;
	ExprFunc(Scope *b, DeclList *d, ExprId *i = nullptr) : body_(b), decls_(d), id_(i) {
		body_->parent_ = this;
	}
	~ExprFunc() {
		delete body_;
		delete decls_;
	}
	Value eval() override;
};

struct ExprQmark : public Expr {
	Value eval() override;
};
struct ExprAssign : public Expr {
private:
	std::unique_ptr<ExprId> id_;
	std::unique_ptr<Expr> expr_;
public:
	ExprAssign(ExprId *i, Expr *e) : id_(i), expr_(e) {
		id_->parent_ = this;
		expr_->parent_ = this;
	}
	Value eval() override;
};

struct ExprApply : public Expr {
private:
	std::unique_ptr<ExprId> id_;
	std::unique_ptr<ExprList> ops_;
public:
	ExprApply(ExprId *i, ExprList *o) : id_(i), ops_(o) {
		id_->parent_ = this;
		for (auto expr : *ops_)
			expr->parent_ = this;
	}
	Value eval() override;
};

struct ExprBinOp : public Expr {
private:
	std::unique_ptr<BinOp> op_;
	std::unique_ptr<Expr> lhs_;
	std::unique_ptr<Expr> rhs_;
public:
	ExprBinOp(BinOp *o, Expr *l, Expr *r) : op_(o), lhs_(l), rhs_(r) {
		op_->parent_ = this;
		lhs_->parent_ = this;
		rhs_->parent_ = this;
	}
	Value eval() override;
};

struct ExprUnOp : public Expr {
private:
	std::unique_ptr<UnOp> op_;
	std::unique_ptr<Expr> rhs_;
public:
	ExprUnOp(UnOp *o, Expr *r) : op_(o), rhs_(r) {
		op_->parent_ = this;
		rhs_->parent_ = this;
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
}
