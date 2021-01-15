#pragma once
#include "inode.hh"
namespace AST {

struct IExecable : public INode {
	virtual void exec() = 0;
};

}
