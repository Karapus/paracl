#include "ast.hh"

namespace AST {

struct Func;

IValue *IValue::defaultValue() {
	return new IntValue{0};
}

void Expr::exec() {
	Value res;
	try {
		res = eval();
	} catch (std::logic_error& err)
	{
		std::cout << "Semantic error: " << err.what() << std::endl;
	}
}

Value BlockList::eval() {
	Value res;
	for (auto expr : cner_) {
		res = expr->eval();
	}
	return res;
}

Value Scope::eval() {
	if (blocks)
		return blocks->eval();
	return Value{};
}

bool Scope::assign(const std::string &name, Value new_val, bool forse_flag) {
	auto dest_it = vars_.find(name);
	if (dest_it != vars_.end()) {
		dest_it->second = new_val;
		return true;
	}
	else if (forse_flag)
		vars_[name] = new_val;
	return forse_flag;
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

Value StmExpr::eval() {
	return expr_->eval();
}

Value StmPrint::eval() {
	auto val = expr_->eval();
	std::cout << static_cast<int>(val) << std::endl;
	return val;
}

Value StmWhile::eval() {
	Value res;
	while (expr_->eval()) {
		res = block_->eval();
	}
	return res;
}

Value StmIf::eval() {
	if (expr_->eval())
		return true_block_->eval();
	if (false_block_)
		return false_block_->eval();
	return Value{};
}

Value StmReturn::eval() {
	throw ReturnExcept(expr_->eval());
}

Value ExprInt::eval() {
	return Value{val_};
}

Value ExprId::eval() {
	for (auto scope = getScope(); scope; scope = scope->getScope()) {
		auto val = scope->resolve(name_);
		if (val)
			return *val;
	}
	return Value();
}

Value Func::operator () (ExprList *ops) {
	auto prev_vars = body_->vars_;
	if (ops->size() != decls_->size())
		throw std::logic_error("incorrect number of arguments in application");
	auto it = decls_->begin();
	for (auto expr : *ops)
		body_->vars_[*it++] = expr->eval();
	Value res;
	try {
		res = body_->eval();
	} catch (ReturnExcept &ret) {
		res = ret.val_;
	}
	body_->vars_ = prev_vars;
	return res;
}

Value ExprFunc::eval() {
	auto val = Value(this);
	if (id_) {
		auto node = parent_;
		while (node->parent_)
			node = node->parent_;
		static_cast<Scope *>(node)->assign(id_->name_, val);
		delete id_;
		id_ = nullptr;
	}
	return val;
}

Value ExprQmark::eval() {
	int val;
	std::cin >> val;
	return Value{val};
}

Value ExprAssign::eval() {
	Value val = expr_->eval();
	const auto &name = id_->name_;
	bool flag;
	for (auto scope = getScope(); scope && (flag = !scope->assign(name, val, false)); scope = scope->getScope())
		;
	if (flag) getScope()->assign(name, val);
	return  val;
}

Value ExprApply::eval() {
	return static_cast<Func>(id_->eval())(ops_.get());
}

Value ExprBinOp::eval() {
	return (*op_)(lhs_.get(), rhs_.get());
}

Value ExprUnOp::eval() {
	return (*op_)(rhs_.get());
}

Func::Func(ExprFunc *func) : body_(func->body_), decls_(func->decls_) 
{}
}
