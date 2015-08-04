#ifndef BreakStatmentNode_h
#define BreakStatmentNode_h

#include "StatementNode.h"

namespace escargot {

class BreakStatementNode : public StatementNode {
public:
    friend class ESScriptParser;
    BreakStatementNode()
            : StatementNode(NodeType::ReturnStatement)
    {
    }

    virtual ESValue execute(ESVMInstance* instance);
};

}

#endif
