#pragma once
#include "location.hh"
#include <ostream>
#include <variant>

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

	virtual operator double() const = 0;
	virtual operator double&() & = 0;

	virtual operator Func() const = 0;
	virtual operator Func&() & = 0;
	virtual IValue *clone() const = 0;
	virtual void free() {
		delete this;
	}
	virtual ~IValue() = default;
};

struct LocIValue : public IValue{
protected:
	LocT origin_;
	LocIValue(LocT loc) : origin_(loc) {
	}
public:
	virtual operator int() const {
		throw IncorrectTypeExcept{origin_};
	}
	virtual operator int&() & {
		throw IncorrectTypeExcept{origin_};
	}
	virtual operator double() const {
		throw IncorrectTypeExcept{origin_};
	}
	virtual operator double&() & {
		throw IncorrectTypeExcept{origin_};
	}
	virtual operator Func() const {
		throw IncorrectTypeExcept{origin_};
	}
	virtual operator Func&() & {
		throw IncorrectTypeExcept{origin_};
	}
};

struct IntValue : public LocIValue {
private:
	int val_;
public:
	operator int() const override {
		return val_;
	}
	operator int&() & override {
		return val_;
	}
	operator double() const override {
		return val_;
	}
	IntValue *clone() const override {
		return new IntValue(origin_, val_);
	}
	IntValue(LocT loc, int val) : LocIValue(loc), val_(val) {
	}
};

struct FloatValue : public LocIValue {
private:
	double val_;
public:
	operator double() const override {
		return val_;
	}
	operator double&() & override {
		return val_;
	}
	operator int() const override {
		return val_;
	}
	FloatValue *clone() const override {
		return new FloatValue(origin_, val_);
	}
	FloatValue(LocT loc, double val) : LocIValue(loc), val_(val) {
	}
};


struct FuncValue : public LocIValue {
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
		return new FuncValue{origin_, func_};
	}
	FuncValue(LocT loc, Func f)  : LocIValue(loc), func_(f) {
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
	operator double() const override {
		throw UdefValExcept{};
	}
	operator double&() & override {
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
	template <typename T>
	operator T() const {
		return static_cast<T>(*ptr_);
	}
	template <typename T>
	operator decltype(static_cast<T&>(*ptr_))() & {
		return static_cast<T&>(*ptr_);
	}
	operator bool() const {
		return static_cast<int>(*ptr_);
	}
	~Value() {
		ptr_->free();
	}
};
}
