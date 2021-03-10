#include "ast.hh"
#include <csetjmp>
#include <utility>
#include <cassert>
#include <iostream>

namespace AST {

struct Context {
	std::vector<VarsT> scope_stack;
	const Expr *prev;
	std::vector<Value> res;
	void operator() (const Expr *expr);
};

void Context::operator() (const Expr *expr) {
	auto root_par = static_cast<const Expr *>(expr->parent_);
	prev = root_par;
	while (expr != root_par) {
		auto tmp = expr->eval(*this);
		prev = expr;
		expr = tmp;
	}
	assert(res.size() == 1);
}

void Expr::exec() {
	Context ctxt;
	try {
		ctxt(this);
	} catch (std::logic_error& err) {
		std::cout << "Semantic error: " << err.what() << std::endl;
	}
	//uncatched return;
}

const Expr *BlockList::eval(Context &ctxt) const {
	Value result;
	for (auto &&expr : cner_) {
		ctxt(expr);
		result = ctxt.res.back();
		ctxt.res.pop_back();
	}
	ctxt.res.push_back(result);
	return static_cast<const Expr *>(parent_);
}

const Expr *Scope::eval(Context &ctxt) const {
	if (blocks) {
		if (ctxt.prev == parent_) {
			ctxt.scope_stack.emplace_back();
			return blocks;
		}
		ctxt.scope_stack.pop_back();
		return static_cast<const Expr *>(parent_);
	} else {
		if (ctxt.prev == parent_)
			ctxt.res.emplace_back();
		return static_cast<const Expr *>(parent_);
	}
}

const Expr *StmPrint::eval(Context &ctxt) const {
	if (ctxt.prev == parent_)
		return expr_.get();
	std::cout << static_cast<int>(ctxt.res.back()) << std::endl;
	return static_cast<const Expr *>(parent_);
}

const Expr *StmWhile::eval(Context &ctxt) const {
	if (ctxt.prev == parent_) {
		ctxt.res.emplace_back();
		return expr_.get();
	}
	if (ctxt.prev == block_.get()) {
		return expr_.get();
	}
	bool flag = ctxt.res.back();
	ctxt.res.pop_back();
	if (flag && block_.get()) {
		ctxt.res.pop_back();
		return block_.get();
	}
	return static_cast<const Expr *>(parent_);
}

const Expr *StmIf::eval(Context &ctxt) const {
	if (ctxt.prev == parent_)
		return expr_.get();
	if (ctxt.prev == expr_.get()) {
		bool is_true = ctxt.res.back();
		ctxt.res.pop_back();
		if (is_true)
			return true_block_.get();
		if (false_block_)
			return false_block_.get();
		ctxt.res.emplace_back();
	}
	return static_cast<const Expr *>(parent_);
}

const Expr *StmReturn::eval(Context &ctxt) const {
#if 0
	expr_->eval(ctxt);
	std::longjmp(ctxt.ret_buf, 1);
#endif
	return nullptr;
}

const Expr *ExprInt::eval(Context &ctxt) const {
	ctxt.res.emplace_back(val_);
	return static_cast<const Expr *>(parent_);
}
const Expr *ExprId::eval(Context &ctxt) const {
	for (auto it = ctxt.scope_stack.rbegin(), end = ctxt.scope_stack.rend(); it != end; ++it) {
		auto var = it->find(name_);
		if (var != it->end()) {
			ctxt.res.push_back(var->second);
			return static_cast<const Expr *>(parent_);
		}
	}
	ctxt.res.emplace_back();
	return static_cast<const Expr *>(parent_);
}

void Func::apply(Context &ctxt, ExprList *ops) {
#if 0
	if (ops->size() != decls_->size())
		throw std::logic_error("incorrect number of arguments in application");
	ctxt.scope_stack.emplace_back();
	auto it = decls_->begin();
	for (auto &&expr : *ops) {
		expr->eval(ctxt);
		ctxt.scope_stack.back().emplace(*it++, ctxt.ret);
	}
	if (!setjmp(ctxt.ret_buf))
		body_->eval(ctxt);
	ctxt.scope_stack.pop_back();
#endif
}

const Expr *ExprFunc::eval(Context &ctxt) const {
#if 0
	ctxt.ret = Func{*this};
	if (id_)
		ctxt.scope_stack.front()[id_->name_] = ctxt.ret;
#endif
	return nullptr;
}

const Expr *ExprQmark::eval(Context &ctxt) const {
	int val;
	std::cin >> val;
	ctxt.res.emplace_back(val);
	return static_cast<const Expr *>(parent_);
}
const Expr *ExprAssign::eval(Context &ctxt) const {
	if (ctxt.prev == parent_)
		return expr_.get();
	auto &&name = id_->name_;
	auto val = ctxt.res.back();
	auto pred  = [val, name](auto &&vars) mutable {
		if (vars.find(name) != vars.end()) {
			vars[name] = std::move(val);
			return false;
		}
		return true;
	};
	if (std::all_of(ctxt.scope_stack.rbegin(), ctxt.scope_stack.rend(), pred))
		ctxt.scope_stack.back()[name] = std::move(val);
	return static_cast<const Expr *>(parent_);
}

const Expr *ExprApply::eval(Context &ctxt) const {
#if 0
	id_->eval(ctxt);
	static_cast<Func>(ctxt.ret).apply(ctxt, ops_.get());
#endif
	return nullptr;
}

const Expr *ExprBinOp::eval(Context &ctxt) const {
	if (ctxt.prev == parent_)
		return lhs_.get();
	if (ctxt.prev == lhs_.get())
		return rhs_.get();
	auto r = std::move(ctxt.res.back());
	ctxt.res.pop_back();
	auto l = std::move(ctxt.res.back());
	ctxt.res.pop_back();
	ctxt.res.emplace_back((*op_)(l, r));
	return static_cast<const Expr *>(parent_);
}

const Expr *ExprUnOp::eval(Context &ctxt) const {
	if (ctxt.prev == parent_)
		return rhs_.get();
	auto r = ctxt.res.back();
	ctxt.res.pop_back();
	ctxt.res.emplace_back((*op_)(r));
	return static_cast<const Expr *>(parent_);
}
}
