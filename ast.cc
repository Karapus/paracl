#include "ast.hh"
#include <csetjmp>
#include <utility>

namespace AST {

struct Context {
	std::vector<VarsT> scope_stack;
	std::jmp_buf ret_buf;
	Value ret;
};


void Expr::exec() {
	Context ctxt;
	try {
		eval(ctxt);
	} catch (std::logic_error& err) {
		std::cout << "Semantic error: " << err.what() << std::endl;
	}
	//uncatched return;
}

void BlockList::eval(Context &ctxt) {
	for (auto &&expr : cner_)
		expr->eval(ctxt);
}

void Scope::eval(Context &ctxt) {
	if (blocks) {
		ctxt.scope_stack.emplace_back();
		blocks->eval(ctxt);
		ctxt.scope_stack.pop_back();
	}
	else
		ctxt.ret = Value{};
}
#if 0
void Scope::assign(const std::string &name, Value new_val) {
	vars_[name] = new_val;
}

bool Scope::assignIfPresent(const std::string &name, Value new_val) {
	auto it = vars_.find(name);
	if (it != vars_.end()) {
		it->second = new_val;
		return true;
	}
	return false;
}

std::optional<Value> Scope::resolve(const std::string &name) const {
	auto it = vars_.find(name);
	return it != vars_.cend() ? std::make_optional(it->second) : std::optional<Value>();
}

Scope *Expr::getScope() {
	auto node = parent_;
	while (node && (typeid(*node) != typeid(Scope)))
		node = node->parent_;
	return static_cast<Scope *>(node);
}

Scope *Expr::getGlobalScope() {
	auto node = parent_;
	while (node->parent_)
		node = node->parent_;
	return static_cast<Scope *>(node);
}
#endif
void StmExpr::eval(Context &ctxt) {
	expr_->eval(ctxt);
}

void StmPrint::eval(Context &ctxt) {
	expr_->eval(ctxt);
	std::cout << static_cast<int>(ctxt.ret) << std::endl;
}

void StmWhile::eval(Context &ctxt) {
	auto res = Value{};
	while (expr_->eval(ctxt), ctxt.ret) {
		block_->eval(ctxt);
		res = ctxt.ret;
	}
	ctxt.ret = res;
}

void StmIf::eval(Context &ctxt) {
	if (expr_->eval(ctxt), ctxt.ret)
		true_block_->eval(ctxt);
	else if (false_block_)
		false_block_->eval(ctxt);
	else
		ctxt.ret = Value{};
}

void StmReturn::eval(Context &ctxt) {
	expr_->eval(ctxt);
	std::longjmp(ctxt.ret_buf, 1);
}

void ExprInt::eval(Context &ctxt) {
	ctxt.ret = Value{val_};
}

void ExprId::eval(Context &ctxt) {
	for (auto it = ctxt.scope_stack.rbegin(), end = ctxt.scope_stack.rend(); it != end; ++it) {
		auto var = it->find(name_);
		if (var != it->end())
			return (void) (ctxt.ret = var->second);
	}
	ctxt.ret  = Value{};
}

void Func::apply(Context &ctxt, ExprList *ops) {
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
}

void ExprFunc::eval(Context &ctxt) {
	ctxt.ret = Func{*this};
	if (id_)
		ctxt.scope_stack.front()[id_->name_] = ctxt.ret;
}

void ExprQmark::eval(Context &ctxt) {
	int val;
	std::cin >> val;
	ctxt.ret = Value{val};
}

void ExprAssign::eval(Context &ctxt) {
	expr_->eval(ctxt);
	auto &&name = id_->name_;
	auto pred  = [&](auto &&vars) mutable {
		if (vars.find(name) != vars.end()) {
			vars[name] = ctxt.ret;
			return false;
		}
		return true;
	};
	if (std::all_of(ctxt.scope_stack.rbegin(), ctxt.scope_stack.rend(), pred))
		ctxt.scope_stack.back()[name] = ctxt.ret;
}

void ExprApply::eval(Context &ctxt) {
	id_->eval(ctxt);
	static_cast<Func>(ctxt.ret).apply(ctxt, ops_.get());
}

void ExprBinOp::eval(Context &ctxt) {
	auto l = (lhs_->eval(ctxt), ctxt.ret);
	auto r = (rhs_->eval(ctxt), ctxt.ret);
	ctxt.ret = (*op_)(l, r);
}

void ExprUnOp::eval(Context &ctxt) {
	ctxt.ret = (*op_)(
		(rhs_->eval(ctxt), ctxt.ret)
		);
}
}
