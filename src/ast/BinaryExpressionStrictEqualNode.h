#ifndef BinaryExpressionStrictEqualNode_h
#define BinaryExpressionStrictEqualNode_h

#include "ExpressionNode.h"

namespace escargot {

class BinaryExpressionStrictEqualNode : public ExpressionNode {
public:
    friend class ESScriptParser;

    BinaryExpressionStrictEqualNode(Node *left, Node* right)
            : ExpressionNode(NodeType::BinaryExpressionStrictEqual)
    {
        m_left = (ExpressionNode*)left;
        m_right = (ExpressionNode*)right;
    }

    ESValue execute(ESVMInstance* instance)
    {
        return ESValue(m_left->execute(instance).equalsTo(m_right->execute(instance)));
    }

protected:
    ExpressionNode* m_left;
    ExpressionNode* m_right;
};

}

#endif