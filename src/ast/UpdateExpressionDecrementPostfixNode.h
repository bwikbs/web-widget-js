#ifndef UpdateExpressionDecrementPostfixNode_h
#define UpdateExpressionDecrementPostfixNode_h

#include "ExpressionNode.h"

namespace escargot {

class UpdateExpressionDecrementPostfixNode : public ExpressionNode {
public:
    friend class ESScriptParser;

    UpdateExpressionDecrementPostfixNode(Node *argument)
            : ExpressionNode(NodeType::UpdateExpressionDecrementPostfix)
    {
        m_argument = (ExpressionNode*)argument;
        m_isSimpleCase = false;
    }

    virtual void generateExpressionByteCode(CodeBlock* codeBlock, ByteCodeGenerateContext& context)
    {
        if(m_isSimpleCase) {
            m_argument->generateResolveAddressByteCode(codeBlock, context);
            m_argument->generateReferenceResolvedAddressByteCode(codeBlock, context);
            codeBlock->pushCode(ToNumber(), this);
            codeBlock->pushCode(Decrement(), this);
            m_argument->generatePutByteCode(codeBlock, context);
            return ;
        }
        m_argument->generateResolveAddressByteCode(codeBlock, context);
        m_argument->generateReferenceResolvedAddressByteCode(codeBlock, context);
        codeBlock->pushCode(ToNumber(), this);
        codeBlock->pushCode(DuplicateTopOfStackValue(), this);
        codeBlock->pushCode(PushIntoTempStack(), this);
        codeBlock->pushCode(Decrement(), this);
        m_argument->generatePutByteCode(codeBlock, context);
        codeBlock->pushCode(Pop(), this);
        codeBlock->pushCode(PopFromTempStack(), this);
    }

protected:
    ExpressionNode* m_argument;
    bool m_isSimpleCase;
};

}

#endif
