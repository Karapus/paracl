#pragma once
#include "location.hh"
#include <ostream>

namespace AST {

using LocT = yy::location;
struct Scope;
struct DeclList;

struct Func {
	const Scope *body_;
	const DeclList *decls_;
	Func(const Scope *body, const DeclList *decls_) : body_(body), decls_(decls_) {
	}
};

namespace Values {

struct ValueExcept {
	virtual void print(std::ostream& os) const = 0; 
	virtual ~ValueExcept() = default;
};

inline std::ostream& operator << (std::ostream& os, const ValueExcept &err) {
	err.print(os);
	return os;
}

struct UdefValExcept : ValueExcept {
	void print(std::ostream& os) const override {
		os << "Undefined value";
	}
};

struct IncorrectTypeExcept : ValueExcept {
private:
	LocT origin_;
public:
	void print(std::ostream& os) const override {
		os << "Value of incorrect type declared at " << origin_;
	}
	IncorrectTypeExcept(LocT l) : origin_(l) {
	}
};

struct IValue {
	virtual operator int() const = 0;
	virtual operator int&() & = 0;
	virtual operator Func() const = 0;
	virtual operator Func&() & = 0;
	virtual IValue *clone() const = 0;
	virtual void free() {
		delete this;
	}
	virtual ~IValue() = default;
};

struct IntValue : public IValue {
private:
	LocT origin_;
	int val_;
public:
	operator int() const override {
		return val_;
	}
	operator int&() & override {
		return val_;
	}
	operator Func() const override {
		throw IncorrectTypeExcept{origin_};
	}
	operator Func&() & override {
		throw IncorrectTypeExcept{origin_};
	}
	IntValue *clone() const override {
		return new IntValue(origin_, val_);
	}
	IntValue(LocT loc, int val) : origin_(loc), val_(val) {
	}
};


struct FuncValue : public IValue {
private:
	LocT origin_;
	Func func_;
public:
	operator int() const override {
		throw IncorrectTypeExcept{origin_};
	}
	operator int&() & override {
		throw IncorrectTypeExcept{origin_};
	}
	operator Func() const override {
		return func_;
	}
	operator Func&() & override {
		return func_;
	}
	FuncValue *clone() const override {
		return new FuncValue{origin_, func_};
	}
	FuncValue(LocT loc, Func f)  : origin_(loc), func_(f) {
	}
};

struct DefaultValue : public IValue {
private:
	DefaultValue() = default; 
public:
	operator int() const override {
		throw UdefValExcept{};
	}
	operator int&() & override {
		throw UdefValExcept{};
	}
	operator Func() const override {
		throw UdefValExcept{};
	}
	operator Func&() & override {
		throw UdefValExcept{};
	}
	DefaultValue *clone() const override {
		return instance();
	}
	void free() override {
	}
	static DefaultValue *instance() {
		static auto val_ = DefaultValue{};
		return &val_;
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
	Value(LocT loc, int val) : ptr_(new Values::IntValue(loc, val)) {
	}
	Value(LocT loc, Func val) : ptr_(new Values::FuncValue(loc, val)) {
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
