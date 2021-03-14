#pragma once
#include "ast.hh"
namespace AST {

struct Context {
	using ScopeStackT = std::vector<VarsT>;
	ScopeStackT scope_stack;
	std::vector<std::pair<const ExprApply *, ScopeStackT>> call_stack;
	const Expr *prev = nullptr;
	std::vector<Value> res;
};

void exec(const INode *root);
}
