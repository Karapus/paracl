#pragma once
#include "inode.h"
namespace AST {

class IExecable : public INode {
public:
	virtual void exec() = 0;
};

}
