#ifndef UnaryExpressionBitwiseNotNode_h
#define UnaryExpressionBitwiseNotNode_h

#include "ExpressionNode.h"

namespace escargot {

class UnaryExpressionBitwiseNotNode : public ExpressionNode {
public:
    friend class ESScriptParser;
    UnaryExpressionBitwiseNotNode(Node* argument)
        : ExpressionNode(NodeType::UnaryExpressionBitwiseNot)
    {
        m_argument = argument;
    }

    ESValue execute(ESVMInstance* instance)
    {
        return ESValue(~m_argument->execute(instance).toInt32());
    }
protected:
    Node* m_argument;
};

}

#endif