#include "exec.hh"
#include <cassert>
#include <iostream>
#include <new>
#include <stdexcept>

namespace AST {

void exec(const INode *root) {
	auto expr = static_cast<const Expr *>(root);
	Context ctxt;
	try {
		while (expr) {
			auto tmp = expr->eval(ctxt);
			ctxt.prev = expr;
			expr = tmp;
		}
		assert(ctxt.res.size() == 1);
	} catch (const std::logic_error& err) {
		std::cout << "Semantic error: " << err.what() << std::endl;
	} catch (const std::bad_alloc& ba) {
		std::cout << "Context is too large: " << ba.what() << std::endl;
	}
}

const Expr *Empty::eval(Context &ctxt) const {
	ctxt.res.emplace_back();
	return parent_;
}

const Expr *Scope::eval(Context &ctxt) const {
	if (ctxt.prev == blocks) {
		if (blocks)
			ctxt.scope_stack.pop_back();
		return parent_;
	}
	if (blocks) {
		ctxt.scope_stack.emplace_back();
		return blocks;
	}
	ctxt.res.emplace_back();
	return parent_;
}

const Expr *Seq::eval(Context &ctxt) const {
	if (ctxt.prev == parent_)
		return fst_.get();
	if (ctxt.prev == fst_.get()) {
		ctxt.res.pop_back();
		return snd_.get();
	}
	return parent_;
}

const Expr *While::eval(Context &ctxt) const {
	if (ctxt.prev == parent_)
		return expr_.get();
	if (ctxt.prev == block_.get()) {
		ctxt.res.pop_back();
		return expr_.get();
	}
	bool flag = ctxt.res.back();
	if (flag && block_) {
		ctxt.res.pop_back();
		return block_.get();
	}
	return parent_;
}

const Expr *If::eval(Context &ctxt) const {
	if (ctxt.prev == parent_)
		return expr_.get();
	if (ctxt.prev == expr_.get()) {
		bool flag = ctxt.res.back();
		ctxt.res.pop_back();
		if (flag)
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

const Expr *Return::eval(Context &ctxt) const {
	if (ctxt.prev == parent_)
		return expr_.get();
	if (ctxt.call_stack.empty())
		throw std::logic_error("Return without call");
	return ctxt.call_stack.back().first;
}

const Expr *ExprFunc::eval(Context &ctxt) const {
	if (ctxt.prev == parent_) {
		ctxt.res.emplace_back(Func{*const_cast<ExprFunc *>(this)});
		if (id_)
			ctxt.scope_stack.front()[id_->name_] = ctxt.res.back();
		return parent_;
	}
	return ctxt.call_stack.back().first;
}

const Expr *ExprApply::eval(Context &ctxt) const {
	if (ctxt.prev == parent_) {
		if (ops_)
			return ops_.get();
		return id_.get();
	}
	if (ctxt.prev == ops_.get())
		return id_.get();
	if (ctxt.prev == id_.get()) {
		Func func = ctxt.res.back();
		ctxt.res.pop_back();
		if ((ops_ ? ops_->size() : 0) != func.decls_->size())
			throw std::logic_error("Incorrect number of arguments");
		ctxt.call_stack.emplace_back(this, std::move(ctxt.scope_stack));
		ctxt.scope_stack = {ctxt.call_stack.back().second.front(), VarsT{}};	//emplace only the global scope and func scope
		auto &&func_scope = ctxt.scope_stack.back();
		auto res_it = ctxt.res.rbegin();
		for (auto it = func.decls_->cbegin(), end = func.decls_->cend(); it != end; ++it)
			func_scope.emplace(*it, *res_it++);
		ctxt.res.erase(res_it.base(), ctxt.res.end());
		return func.body_;
	}
	ctxt.call_stack.back().second.front() = std::move(ctxt.scope_stack.front());
	ctxt.scope_stack = std::move(ctxt.call_stack.back().second);
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
