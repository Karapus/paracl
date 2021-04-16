#pragma once
#include "ast.hh"
#include <string>
namespace AST {

template <typename T, typename... Args>
INode *make(Args... args) {
	return new T{args...};
}

template <>
inline INode *make<DeclList>(INode *declist, INode *id) {
	static_cast<DeclList *>(declist)->push_back(std::move(static_cast<ExprId *>(id)->name_));
	delete id;
	return declist;
}

template <>
inline INode *make<ExprList>() {
	return nullptr;
}
}
