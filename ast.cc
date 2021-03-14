#include "ast.hh"
#include <csetjmp>
#include <utility>
#include <cassert>
#include <iostream>

namespace AST {

struct Context {
	std::vector<VarsT> scope_stack;
	std::vector<const ExprApply *> call_stack;
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
//	assert(res.size() == 1);
}

void Expr::exec() {
	Context ctxt;
	try {
		ctxt(this);
	} catch (std::logic_error& err) {
		std::cout << "Semantic error: " << err.what() << std::endl;
	}
}

const Expr *BlockList::eval(Context &ctxt) const {
	if (ctxt.prev == parent_) {
		if (blocks_)
			return blocks_.get();
		ctxt.res.pop_back();
		return block_.get();
	}
	if (ctxt.prev == blocks_.get()) {
		ctxt.res.pop_back();
		return block_.get();
	}
	return parent_;
}

const Expr *Scope::eval(Context &ctxt) const {
	if (ctxt.prev == blocks) {
		if (blocks)
			ctxt.scope_stack.pop_back();
		return parent_;
	}
	ctxt.res.emplace_back();
	if (blocks) {
		ctxt.scope_stack.emplace_back();
		return blocks;
	}
	return parent_;
}

const Expr *StmPrint::eval(Context &ctxt) const {
	if (ctxt.prev == parent_)
		return expr_.get();
	std::cout << static_cast<int>(ctxt.res.back()) << std::endl;
	return parent_;
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
	if (flag && block_) {
		ctxt.res.pop_back();
		return block_.get();
	}
	return parent_;
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
	return parent_;
}

const Expr *ExprInt::eval(Context &ctxt) const {
	ctxt.res.emplace_back(val_);
	return parent_;
}
const Expr *ExprId::eval(Context &ctxt) const {
	for (auto it = ctxt.scope_stack.rbegin(), end = ctxt.scope_stack.rend(); it != end; ++it) {
		auto var = it->find(name_);
		if (var != it->end()) {
			ctxt.res.push_back(var->second);
			return parent_;
		}
	}
	ctxt.res.emplace_back();
	return parent_;
}

const Expr *ExprList::eval(Context &ctxt) const {
	if (ctxt.prev == parent_) 
		return head_.get();
	if (ctxt.prev == head_.get()) {
		if (tail_)
			return tail_.get();
	}
	return parent_;
}

const Expr *StmReturn::eval(Context &ctxt) const {
	if (ctxt.prev == parent_) {
		//ctxt.res.pop_back();
		return expr_.get();
	}
	return ctxt.call_stack.back();
}

const Expr *ExprFunc::eval(Context &ctxt) const {
	if (ctxt.prev == parent_) {
		ctxt.res.emplace_back(Func{*const_cast<ExprFunc *>(this)});
		if (id_)
			ctxt.scope_stack.front()[id_->name_] = ctxt.res.back();
		return parent_;
	}
	return ctxt.call_stack.back();
}

const Expr *ExprApply::eval(Context &ctxt) const {
	if (ctxt.prev == parent_) {
		if (ops_.get())
			return ops_.get();
		return id_.get();
	}
	if (ctxt.prev == ops_.get())
		return id_.get();
	if (ctxt.prev == id_.get()) {
		Func func = ctxt.res.back();
		ctxt.res.pop_back();
		ctxt.call_stack.push_back(this);
		ctxt.scope_stack.emplace_back();
		auto &&func_scope = ctxt.scope_stack.back();
		for (auto &&decl : *func.decls_) {
			func_scope.emplace(decl, ctxt.res.back());
			ctxt.res.pop_back();
		}
		return func.body_;
	}
	ctxt.call_stack.pop_back();
	return parent_;
}

const Expr *ExprQmark::eval(Context &ctxt) const {
	int val;
	std::cin >> val;
	ctxt.res.emplace_back(val);
	return parent_;
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
	return parent_;
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
	return parent_;
}

const Expr *ExprUnOp::eval(Context &ctxt) const {
	if (ctxt.prev == parent_)
		return rhs_.get();
	auto r = ctxt.res.back();
	ctxt.res.pop_back();
	ctxt.res.emplace_back((*op_)(r));
	return parent_;
}
}
