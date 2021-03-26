#pragma once
#include <stdexcept>

namespace AST {

struct Scope;
struct DeclList;

struct Func {
	const Scope *body_;
	const DeclList *decls_;
	Func(const Scope *body, const DeclList *decls_) : body_(body), decls_(decls_) {
	}
};

namespace Values {
struct IncorrectTypeExcept : public std::logic_error {
	IncorrectTypeExcept() : std::logic_error("Incorrect Type") {
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
	virtual void free() {
		delete this;
	}
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
	IntValue(int val) : val_(val) {
	}
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
	FuncValue(Func f)  : func_(f) {
	}
};

struct DefaultValue : public IValue {
private:
	DefaultValue() = default;
public:
	DefaultValue *clone() const override {
		return instance();
	}
	void free() override {
	}
	static DefaultValue *instance() {
		static auto val_ = new DefaultValue{};
		return val_;
	}
	DefaultValue(const DefaultValue &) = delete;
	DefaultValue &operator = (const DefaultValue &) = delete;
};
} //namespace Values

struct Value {
private:
	Values::IValue *ptr_;
public:
	Value() : ptr_(Values::DefaultValue::instance()) {
	}
	Value(const Value &rhs) : ptr_(rhs.ptr_->clone()) {
	}
	Value(Value &&rhs) noexcept {
		ptr_ = rhs.ptr_;
		rhs.ptr_ = Values::DefaultValue::instance();
	}
	Value &operator = (const Value &rhs) {
		if (this != &rhs) {
			ptr_->free();
			ptr_ = rhs.ptr_->clone();
		}
		return *this;
	}
	Value &operator = (Value &&rhs) noexcept {
		std::swap(ptr_, rhs.ptr_);
		return *this;
	}
	explicit Value(int val) : ptr_(new Values::IntValue(val)) {
	}
	explicit Value(Func val) : ptr_(new Values::FuncValue(val)) {
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
		ptr_->free();
	}
};
}
