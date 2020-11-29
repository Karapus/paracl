#pragma once
#include "inode.h"
namespace AST {

struct IExecable : public INode {
	virtual void exec() = 0;
};

}
