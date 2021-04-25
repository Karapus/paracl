#pragma once
#include "location.hh"
#include <optional>
#include <ostream>
#include <type_traits>
#include <typeinfo>
#include <utility>

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

template <typename T>
struct Val : public IValue {
private:
	LocT origin_;
	T val_;
public:
	operator int() const override {
		throw IncorrectTypeExcept{origin_};
	}
	operator int&() & override {
		throw IncorrectTypeExcept{origin_};
	}
	operator double() const override {
		throw IncorrectTypeExcept{origin_};
	}
	operator double&() & override {
		throw IncorrectTypeExcept{origin_};
	}
	operator Func() const override {
		throw IncorrectTypeExcept{origin_};
	}
	operator Func&() & override {
		throw IncorrectTypeExcept{origin_};
	}
	Val *clone() const override {
		return new Val(origin_, val_);
	}
	Val(LocT loc, T val) : origin_(loc), val_(val) {
	}
};

template <typename T>
Val(LocT, T) -> Val<T>;

template<>
inline Val<int>::operator double() const {
	return val_;
}

template<>
inline Val<int>::operator int() const {
	return val_;
}

template<>
inline Val<int>::operator int&() & {
	return val_;
}

template<>
inline Val<double>::operator int() const {
	return val_;
}

template<>
inline Val<double>::operator double() const {
	return val_;
}

template<>
inline Val<double>::operator double&() & {
	return val_;
}

template<>
inline Val<Func>::operator Func() const {
	return val_;
}

template<>
inline Val<Func>::operator Func&() & {
	return val_;
}

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
	template <typename T>
	Value(LocT loc, T val) : ptr_(new Values::Val(loc, val)) {
	}
	template <typename T>
	operator T() const {
		return ptr_->operator T();
	}
	template <typename T>
	operator decltype(ptr_->operator T&)() & {
		return ptr_->operator T&();
	}
	operator bool() const {
		return operator int();
	}
	template <typename T>
	bool isSameType() const {
		return typeid(*ptr_) == typeid(Values::Val<T>);
	}
	~Value() {
		ptr_->free();
	}
};

namespace Values {

struct NoConversionExcept : ValueExcept {
private:
	LocT loc_;
public:
	void print(std::ostream& os) const override {
		os << "Value of incorrect type declared at " << loc_;
	}
	NoConversionExcept(LocT l) : loc_(l) {
	}
};
namespace detail {

template <typename T, typename Arg>
auto get(Arg arg) {
	return arg.template isSameType<T>() ? std::make_optional<Arg>(arg) : std::nullopt;
}

template <typename T, typename Arg, typename... Args>
auto get(Arg arg, Args... args) {
	return arg.template isSameType<T>() ? std::make_optional<Arg>(arg) : get<T>(args...);
}


template <template <typename> typename F, typename T, typename... Args>
auto apply(Args... args) {
	auto res = get<T>(args...);
	if (res)
		res->operator T&() = F<T>{}(static_cast<T>(args)...);
	return res;
}

template <template <typename> typename F, typename T, typename... Ts, typename... Args>
auto apply(Args... args) -> decltype(detail::apply<F, Ts...>(args...)) {
	auto res = detail::apply<F, T>(args...);
	return res ? res : detail::apply<F, Ts...>(args...);
}
}
}

template <template <typename> typename F, typename... Ts, typename... Args>
auto apply(Args... args)
	-> decltype(Values::detail::apply<F, Ts...>(args...)) {
	return Values::detail::apply<F, Ts...>(args...);
}

template <template <typename> typename F, typename... Args>
auto apply(Args... args) {
	return Values::detail::apply<F, double, int>(args...);
}
}
