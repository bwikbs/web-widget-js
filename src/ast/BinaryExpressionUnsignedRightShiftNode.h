#ifndef BinaryExpressionUnsignedRightShiftNode_h
#define BinaryExpressionUnsignedRightShiftNode_h

#include "ExpressionNode.h"

namespace escargot {

class BinaryExpressionUnsignedRightShiftNode : public ExpressionNode {
public:
    BinaryExpressionUnsignedRightShiftNode(Node *left, Node* right)
            : ExpressionNode(NodeType::BinaryExpressionUnsignedRightShift)
    {
        m_left = (ExpressionNode*)left;
        m_right = (ExpressionNode*)right;
    }

    ESValue execute(ESVMInstance* instance)
    {
        ESValue lval = m_left->execute(instance);
        ESValue rval = m_right->execute(instance);
        int32_t rnum = rval.toInt32();
        int32_t lnum = lval.toInt32();
        unsigned int shiftCount = ((unsigned int)rnum) & 0x1F;
        lnum = ((unsigned int)lnum) >> shiftCount;

        return ESValue(lnum);
    }
protected:
    ExpressionNode* m_left;
    ExpressionNode* m_right;
};

}

#endif