#pragma once
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>
#include <iostream>

namespace AST {

struct Func;
struct ExprFunc;
struct Value;
struct Scope;
struct DeclList;
struct ExprList;
struct Context;

struct IncorrectTypeExcept : public std::logic_error {
	IncorrectTypeExcept() : std::logic_error("Incorrect Type") {
	}
};

struct Func {
	const Scope *body_;
	const DeclList *decls_;
	Func(const Scope *body, const DeclList *decls_) : body_(body), decls_(decls_) {
	}
};


struct IValue {
	virtual operator int() const {
		throw IncorrectTypeExcept{};
	}
	virtual operator int&() & {
		throw IncorrectTypeExcept{};
	}
	virtual operator Func() const {
		throw IncorrectTypeExcept{};
	}
	virtual operator Func&() & {
		throw IncorrectTypeExcept{};
	}
	virtual IValue *clone() const = 0;
	virtual ~IValue() = default;
};

struct IntValue : public IValue {
private:
	int val_;
public:
	operator int() const override {
		return val_;
	}
	operator int&() & override {
		return val_;
	}
	IntValue *clone() const override {
		return new IntValue(val_);
	}
	IntValue(int val) : val_(val)
	{}
};


struct FuncValue : public IValue {
private:
	Func func_;
public:
	operator Func() const override {
		return func_;
	}
	operator Func&() & override {
		return func_;
	}
	FuncValue *clone() const override {
		return new FuncValue(func_);
	}
	FuncValue(Func f) : func_(f)
	{}
};

struct DefaultValue : public IValue {
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
	Value(const Value &rhs) : ptr_(rhs.ptr_->clone()) {
	}
	Value(Value &&rhs) noexcept {
		ptr_ = rhs.ptr_;
		rhs.ptr_ = nullptr;
	}
	Value &operator = (const Value &rhs) {
		if (this != &rhs) {
			delete ptr_;
			ptr_ = rhs.ptr_->clone();
		}
		return *this;
	}
	Value &operator = (Value &&rhs) noexcept {
		std::swap(ptr_, rhs.ptr_);
		return *this;
	}
	Value(int val) : ptr_(new IntValue(val)) {
	}
	Value(Func val) : ptr_(new FuncValue(val)) {
	}
	operator int&() & {
		return static_cast<int &>(*ptr_);
	}
	operator Func&() & {
		return static_cast<Func &>(*ptr_);
	}
	template <typename T>
	operator T() const {
		return static_cast<T>(*ptr_);
	}
	operator bool() const {
		return static_cast<int>(*ptr_);
	}
	~Value() {
		delete ptr_;
	}
};

using VarsT = std::unordered_map<std::string, Value>;

struct INode {
	virtual ~INode()
	{}
};

struct Expr;

struct Context {
	using ScopeStackT = std::vector<VarsT>;
	ScopeStackT scope_stack;
	std::vector<const Expr *> call_stack;
	std::vector<ScopeStackT> ctxts_stack;
	const Expr *prev = nullptr;
	std::vector<Value> res;
};

struct Node {
	Expr *parent_ = nullptr;
	virtual ~Node() = default;
};

struct Expr : public Node, public INode {
	virtual const Expr *eval(Context &ctxt) const = 0;
};

struct BinOp : public Node, public INode {
	template <typename F>
	void as_int(F functor, int &lhs, int rhs) const {
		lhs = functor(lhs, rhs);
	}
};

struct UnOp : public Node, public INode {
	template <typename F>
	void as_int(F functor, int &val) const {
		val = functor(val);
	}
};

struct BlockList : public Expr {
private:
	std::unique_ptr<BlockList> blocks_;
	std::unique_ptr<Expr> block_;
public:
	BlockList(BlockList *blocks, Expr *block) : blocks_(blocks), block_(block) {
		block_->parent_ = this;
		if (blocks_)
			blocks_->parent_ = this;
	}
	const Expr *eval(Context &ctxt) const override;
};

struct DeclList : public INode {
private:
	std::vector<std::string> cner_;
public:
	auto cbegin() const {
		return cner_.cbegin();
	}
	auto cend() const {
		return cner_.cend();
	}
	auto size() const {
		return cner_.size();
	}
	auto push_back(const std::string &x) {
		return cner_.push_back(x);
	}
};

struct ExprList : public Expr {
private:
	std::unique_ptr<ExprList> tail_;
	std::unique_ptr<Expr> head_;
public:
	ExprList(ExprList *tail, Expr *head) : tail_(tail), head_(head) {
		head_->parent_ = this;
		if (tail_)
			tail_->parent_ = this;
	}
	const Expr *eval(Context &ctxt) const override;
	std::size_t size() const {
		return tail_ ? (tail_->size() + 1) : 1;
	}
};

struct Empty : public Expr {
	const Expr *eval(Context &ctxt) const override; 
};

struct Scope : public Expr {
private:
	BlockList *blocks;
public:
	Scope(BlockList *b) : blocks(b) {
		if (blocks)
			blocks->parent_ = this;
	}
	~Scope() {
		delete blocks;
	}
	const Expr *eval(Context &ctxt) const override; 
};

struct Seq : public Expr {
private:
	std::unique_ptr<Expr> fst_;
	std::unique_ptr<Expr> snd_;
public:
	Seq(Expr *fst, Expr *snd) : fst_(fst), snd_(snd) {
		fst_->parent_ = this;
		snd_->parent_ = this;
	}
	const Expr *eval(Context &ctxt) const override; 
};

struct While : public Expr {
private:
	std::unique_ptr<Expr> expr_;
	std::unique_ptr<Expr> block_;
public:
	While(Expr *e, Expr *b) : expr_(e), block_(b) {
		expr_->parent_ = block_->parent_ = this;
	}
	const Expr *eval(Context &ctxt) const override;
};

struct If : public Expr {
private:
	std::unique_ptr<Expr> expr_;
	std::unique_ptr<Expr> true_block_;
	std::unique_ptr<Expr> false_block_;
public:
	If(Expr *e, Expr *tb, Expr *fb) : expr_(e), true_block_(tb), false_block_(fb) {
		expr_->parent_ = this;
		true_block_->parent_ = this;
		if (false_block_)
			false_block_->parent_ = this;
	}
	const Expr *eval(Context &ctxt) const override;
};

struct Return : public Expr {
private:
	std::unique_ptr<Expr> expr_;
public:
	Return(Expr *expr) : expr_(expr) {
		expr_->parent_ = this;
	}
	const Expr *eval(Context &ctxt) const override;
};

struct ExprInt : public Expr {
private:
	int val_;
public:
	ExprInt(int i) : val_(i)
	{}
	const Expr *eval(Context &ctxt) const override;
};

struct ExprId : public Expr {
	std::string name_;
	ExprId(std::string n) : name_(n)
	{}
	const Expr *eval(Context &ctxt) const override;
};

struct ExprFunc : public Expr {
private:
	std::unique_ptr<Scope> body_;
	std::unique_ptr<DeclList> decls_;
	std::unique_ptr<ExprId> id_;
public:
	ExprFunc(Scope *b, DeclList *d, ExprId *i = nullptr) : body_(b), decls_(d), id_(i) {
		body_->parent_ = this;
	}
	operator Func() {
		return Func{body_.get(), decls_.get()};
	}
	const Expr *eval(Context &ctxt) const override;
};

struct ExprQmark : public Expr {
	const Expr *eval(Context &ctxt) const override;
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
	const Expr *eval(Context &ctxt) const override;
};

struct ExprApply : public Expr {
private:
	std::unique_ptr<ExprId> id_;
	std::unique_ptr<ExprList> ops_;
public:
	ExprApply(ExprId *i, ExprList *o) : id_(i), ops_(o) {
		id_->parent_ = this;
		if (ops_)
			ops_->parent_ = this;
	}
	const Expr *eval(Context &ctxt) const override;
};

template <typename T>
struct ExprBinOp : public Expr {
private:
	T op_;
	std::unique_ptr<Expr> lhs_;
	std::unique_ptr<Expr> rhs_;
public:
	ExprBinOp(Expr *l, Expr *r) : lhs_(l), rhs_(r) {
		lhs_->parent_ = this;
		rhs_->parent_ = this;
	}
	const Expr *eval(Context &ctxt) const {
		if (ctxt.prev == parent_)
			return lhs_.get();
		if (ctxt.prev == lhs_.get())
			return rhs_.get();
		auto r = std::move(ctxt.res.back());
		ctxt.res.pop_back();
		auto l = std::move(ctxt.res.back());
		ctxt.res.pop_back();
		ctxt.res.emplace_back(op_(l, r));
		return parent_;
	}
};

template <typename T>
struct ExprUnOp : public Expr {
private:
	T op_;
	std::unique_ptr<Expr> rhs_;
public:
	ExprUnOp(Expr *r) : rhs_(r) {
		rhs_->parent_ = this;
	}
	const Expr *eval(Context &ctxt) const override {
		if (ctxt.prev == parent_)
			return rhs_.get();
		auto r = std::move(ctxt.res.back());
		ctxt.res.pop_back();
		ctxt.res.emplace_back(op_(r));
		return parent_;
	}
};

struct BinOpMul : public BinOp {
	Value operator() (Value lhs, const Value &rhs) const {
		as_int(std::multiplies<int>{}, lhs, rhs);
		return lhs;
	}
};
struct BinOpDiv : public BinOp {
	Value operator() (Value lhs, const Value &rhs) const {
		as_int(std::divides<int>{}, lhs, rhs);
		return lhs;
	}
};
struct BinOpMod : public BinOp {
	Value operator() (Value lhs, const Value &rhs) const {
		as_int(std::modulus<int>{}, lhs, rhs);
		return lhs;
	}
};
struct BinOpPlus : public BinOp {
	Value operator() (Value lhs, const Value &rhs) const {
		as_int(std::plus<int>{}, lhs, rhs);
		return lhs;
	}
};
struct BinOpMinus : public BinOp {
	Value operator() (Value lhs, const Value &rhs) const {
		as_int(std::minus<int>{}, lhs, rhs);
		return lhs;
	}
};
struct BinOpLess : public BinOp {
	Value operator() (Value lhs, const Value &rhs) const {
		as_int(std::less<int>{}, lhs, rhs);
		return lhs;
	}
};
struct BinOpGrtr : public BinOp {
	Value operator() (Value lhs, const Value &rhs) const {
		as_int(std::greater<int>{}, lhs, rhs);
		return lhs;
	}
};
struct BinOpLessOrEq : public BinOp {
	Value operator() (Value lhs, const Value &rhs) const {
		as_int(std::less_equal<int>{}, lhs, rhs);
		return lhs;
	}
};
struct BinOpGrtrOrEq : public BinOp {
	Value operator() (Value lhs, const Value &rhs) const {
		as_int(std::greater_equal<int>{}, lhs, rhs);
		return lhs;
	}
};
struct BinOpEqual : public BinOp {
	Value operator() (Value lhs, const Value &rhs) const {
		as_int(std::equal_to<int>{}, lhs, rhs);
		return lhs;
	}
};
struct BinOpNotEqual : public BinOp {
	Value operator() (Value lhs, const Value &rhs) const {
		as_int(std::not_equal_to<int>{}, lhs, rhs);
		return lhs;
	}
};
struct BinOpAnd : public BinOp {
	Value operator() (Value lhs, const Value &rhs) const {
		as_int(std::logical_and<int>{}, lhs, rhs);
		return lhs;
	}
};
struct BinOpOr : public BinOp {
	Value operator() (Value lhs, const Value &rhs) const {
		as_int(std::logical_or<int>{}, lhs, rhs);
		return lhs;
	}
};

struct UnOpPlus : public UnOp {
	Value operator() (Value val) const {
		as_int([](int a) { return +a; }, val);
		return val;
	}
};
struct UnOpMinus : public UnOp {
	Value operator() (Value val) const {
		as_int(std::negate<int>{}, val);
		return val;
	}
};
struct UnOpNot : public UnOp {
	Value operator() (Value val) const {
		as_int(std::logical_not<int>{}, val);
		return val;
	}
};
struct UnOpPrint : public UnOp {
	Value operator() (Value val) const {
		as_int([](int a) {std::cout << a << std::endl; return a; }, val);
		return val;
	}
};
}
