#include "ast.hh"

namespace AST {

struct Func;
#if 0
struct IValue {
	virtual operator int&() = 0;
	virtual operator Func&() = 0;
	virtual IValue *clone() const = 0;
	virtual ~IValue() {};
	static IValue *defaultValue();
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

#endif

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
#if 0
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
#endif
	Value BlockList::eval() {
		Value res;
		for (auto expr : cner_) {
			res = expr->eval();
		}
		return res;
	}
#if 0
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

#endif
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
	return expr->eval();
}

Value StmPrint::eval() {
	auto val = expr->eval();
	std::cout << static_cast<int>(val) << std::endl;
	return val;
}

Value StmWhile::eval() {
	Value res;
	while (expr->eval()) {
		res = block->eval();
	}
	return res;
}

Value StmIf::eval() {
	if (expr->eval())
		return true_block->eval();
	if (false_block)
		return false_block->eval();
	return Value{};
}

Value StmReturn::eval() {
	throw ReturnExcept(expr_->eval());
}

Value ExprInt::eval() {
	return Value{val};
}

Value ExprId::eval() {
	for (auto scope = getScope(); scope; scope = scope->getScope()) {
		auto val = scope->resolve(name);
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
	if (id) {
		auto node = parent_;
		while (node->parent_)
			node = node->parent_;
		static_cast<Scope *>(node)->assign(id->name, val);
		delete id;
		id = nullptr;
	}
	return val;
}

Value ExprQmark::eval() {
	int val;
	std::cin >> val;
	return Value{val};
}

Value ExprAssign::eval() {
	Value val = expr->eval();
	const auto &name = id->name;
	bool flag;
	for (auto scope = getScope(); scope && (flag = !scope->assign(name, val, false)); scope = scope->getScope())
		;
	if (flag) getScope()->assign(name, val);
	return  val;
}

Value ExprApply::eval() {
	return static_cast<Func>(id->eval())(ops);
}

Value ExprBinOp::eval() {
	return (*op)(lhs, rhs);
}

Value ExprUnOp::eval() {
	return (*op)(rhs);
}

Func::Func(ExprFunc *func) : body_(func->body), decls_(func->decls) 
{}
}
