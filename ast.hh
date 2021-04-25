#pragma once
#include "value.hh"
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <functional>
#include <memory>
#include <iostream>

namespace AST {

using VarsT = std::unordered_map<std::string, Value>;

struct Expr;

struct INode {
	Expr *parent_ = nullptr;
	virtual ~INode() = default;
};


struct Context {
	using ScopeStackT = std::vector<VarsT>;
	ScopeStackT scope_stack;
	std::vector<const Expr *> call_stack;
	std::vector<ScopeStackT> ctxts_stack;
	const Expr *prev = nullptr;
	std::vector<Value> res;
};

struct Expr : public INode {
	LocT loc_;
	Expr(LocT loc) : loc_(loc) {
	}
	virtual const Expr *eval(Context &ctxt) const = 0;
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
	auto push_back(std::string &&x) {
		return cner_.push_back(x);
	}
};

struct ExprList : public Expr {
private:
	std::unique_ptr<ExprList> tail_;
	std::unique_ptr<Expr> head_;
public:
	ExprList(LocT loc, INode *tail, INode *head) :
		Expr(loc),
		tail_(static_cast<ExprList *>(tail)),
		head_(static_cast<Expr *>(head))
	{
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
	Empty(LocT loc) : Expr(loc) {}
	const Expr *eval(Context &ctxt) const override;
};

struct Scope : public Expr {
private:
	std::unique_ptr<Expr> blocks_;
public:
	Scope(LocT loc, INode *blocks) :
		Expr(loc),
		blocks_(static_cast<Expr *>(blocks))
	{
		blocks->parent_ = this;
	}
	const Expr *eval(Context &ctxt) const override; 
};

struct Seq : public Expr {
private:
	std::unique_ptr<Expr> fst_;
	std::unique_ptr<Expr> snd_;
public:
	Seq(LocT loc, INode *fst, INode *snd) :
		Expr(loc),
		fst_(static_cast<Expr *>(fst)),
		snd_(static_cast<Expr *>(snd))
	{
		fst_->parent_ = snd_->parent_ = this;
	}
	const Expr *eval(Context &ctxt) const override; 
};

struct While : public Expr {
private:
	std::unique_ptr<Expr> expr_;
	std::unique_ptr<Expr> block_;
public:
	While(LocT loc, INode *expr, INode *block) :
		Expr(loc),
		expr_(static_cast<Expr *>(expr)),
		block_(static_cast<Expr *>(block))
	{
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
	If(LocT loc, INode *expr, INode *tb, INode *fb = nullptr) :
		Expr(loc),
		expr_(static_cast<Expr *>(expr)),
		true_block_(static_cast<Expr *>(tb)),
		false_block_(static_cast<Expr *>(fb))
	{
		expr_->parent_ = true_block_->parent_ = this;
		if (false_block_)
			false_block_->parent_ = this;
	}
	const Expr *eval(Context &ctxt) const override;
};

struct Return : public Expr {
private:
	std::unique_ptr<Expr> expr_;
public:
	Return(LocT loc, INode *expr) :
		Expr(loc),
		expr_(static_cast<Expr *>(expr))
	{
		expr_->parent_ = this;
	}
	const Expr *eval(Context &ctxt) const override;
};

struct ExprInt : public Expr {
private:
	int val_;
public:
	ExprInt(LocT loc, int i) :
		Expr(loc),
		val_(i)
	{}
	const Expr *eval(Context &ctxt) const override;
};

struct ExprFloat : public Expr {
private:
	double val_;
public:
	ExprFloat(LocT loc, double d) :
		Expr(loc),
		val_(d)
	{}
	const Expr *eval(Context &ctxt) const override;
};

struct ExprId : public Expr {
	std::string name_;
	ExprId(LocT loc, std::string n) :
		Expr(loc),
		name_(n)
	{}
	const Expr *eval(Context &ctxt) const override;
};

struct ExprFunc : public Expr {
private:
	std::unique_ptr<Scope> body_;
	std::unique_ptr<DeclList> decls_;
	std::unique_ptr<ExprId> id_;
public:
	ExprFunc(LocT loc, INode *body, INode *decls, INode *id = nullptr) :
		Expr(loc),
		body_(static_cast<Scope *>(body)),
		decls_(static_cast<DeclList *>(decls)),
		id_(static_cast<ExprId *>(id))
	{
		body_->parent_ = this;
	}
	const Expr *eval(Context &ctxt) const override;
};

struct ExprQmark : public Expr {
	ExprQmark(LocT loc) : Expr(loc) {}
	const Expr *eval(Context &ctxt) const override;
};

struct ExprAssign : public Expr {
private:
	std::unique_ptr<ExprId> id_;
	std::unique_ptr<Expr> expr_;
public:
	ExprAssign(LocT loc, INode *i, INode *e) :
		Expr(loc),
		id_(static_cast<ExprId *>(i)),
		expr_(static_cast<Expr *>(e))
	{
		id_->parent_ = expr_->parent_ = this;
	}
	const Expr *eval(Context &ctxt) const override;
};

struct ExprApply : public Expr {
private:
	LocT loc_;
	std::unique_ptr<ExprId> id_;
	std::unique_ptr<ExprList> ops_;
public:
	ExprApply(LocT loc, INode *i, INode *o) :
		Expr(loc),
		id_(static_cast<ExprId *>(i)),
		ops_(static_cast<ExprList *>(o))
	{
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
	ExprBinOp(LocT loc, INode *l, INode *r) :
		Expr(loc),
		lhs_(static_cast<Expr *>(l)),
		rhs_(static_cast<Expr *>(r))
	{
		lhs_->parent_ = rhs_->parent_ = this;
	}
	const Expr *eval(Context &ctxt) const {
		if (ctxt.prev == parent_)
			return lhs_.get();
		if (ctxt.prev == lhs_.get())
			return rhs_.get();
		auto r = std::move(ctxt.res.back());
		ctxt.res.pop_back();
		auto&& l = std::move(ctxt.res.back());
		auto&& res = op_(std::move(l), std::move(r));
		if (res)
			ctxt.res.back() = std::move(*res);
		else
			throw Values::NoConversionExcept{loc_};
		return parent_;
	}
};

template <typename T>
struct ExprUnOp : public Expr {
private:
	T op_;
	std::unique_ptr<Expr> rhs_;
public:
	ExprUnOp(LocT loc, INode *r) :
		Expr(loc),
		rhs_(static_cast<Expr *>(r))
	{
		rhs_->parent_ = this;
	}
	const Expr *eval(Context &ctxt) const override {
		if (ctxt.prev == parent_)
			return rhs_.get();
		auto&& res = op_(std::move(ctxt.res.back()));
		if (res)
			ctxt.res.back() = std::move(*res);
		else
			throw Values::NoConversionExcept{loc_};
		return parent_;
	}
};

struct BinOpMul {
	auto operator() (Value lhs, Value rhs) const {
		return apply<std::multiplies>(lhs, rhs);
	}
};
struct BinOpDiv {
	auto operator() (Value lhs, Value rhs) const {
		return apply<std::divides>(lhs, rhs);
	}
};
struct BinOpMod {
	auto operator() (Value lhs, Value rhs) const {
		return apply<std::modulus, int>(lhs, rhs);
	}
};
struct BinOpPlus {
	auto operator() (Value lhs, Value rhs) const {
		return apply<std::plus>(lhs, rhs);
	}
};
struct BinOpMinus {
	auto operator() (Value lhs, Value rhs) const {
		return apply<std::minus>(lhs, rhs);
	}
};
struct BinOpLess {
	auto operator() (Value lhs, Value rhs) const {
		return apply<std::less>(lhs, rhs);
	}
};
struct BinOpGrtr {
	auto operator() (Value lhs, Value rhs) const {
		return apply<std::greater>(lhs, rhs);
	}
};
struct BinOpLessOrEq {
	auto operator() (Value lhs, Value rhs) const {
		return apply<std::less_equal>(lhs, rhs);
	}
};
struct BinOpGrtrOrEq {
	auto operator() (Value lhs, Value rhs) const {
		return apply<std::greater_equal>(lhs, rhs);
	}
};
struct BinOpEqual {
	auto operator() (Value lhs, Value rhs) const {
		return apply<std::equal_to>(lhs, rhs);
	}
};
struct BinOpNotEqual {
	auto operator() (Value lhs, Value rhs) const {
		return apply<std::not_equal_to>(lhs, rhs);
	}
};
struct BinOpAnd {
	auto operator() (Value lhs, Value rhs) const {
		return apply<std::logical_and>(lhs, rhs);
	}
};
struct BinOpOr {
	auto operator() (Value lhs, Value rhs) const {
		return apply<std::logical_or>(lhs, rhs);
	}
};

struct UnOpPlus {
	template <typename T>
	struct Plus {
		auto operator() (T a) { return +a; }
	};
	auto operator() (Value val) const {
		return apply<Plus>(val);
	}
};
struct UnOpMinus {
	auto operator() (Value val) const {
		return apply<std::negate>(val);
	}
};
struct UnOpNot {
	auto operator() (Value val) const {
		return apply<std::logical_not>(val);
	}
};
struct UnOpPrint {
	template <typename T>
	struct Print {
		auto operator() (T a) {
			std::cout << a << std::endl;
			return a;
		}
	};
	auto operator() (Value val) const {
		return apply<Print>(val);
	}
};
}
