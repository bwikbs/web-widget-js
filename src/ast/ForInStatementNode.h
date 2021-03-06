/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifndef ForInStatementNode_h
#define ForInStatementNode_h

#include "StatementNode.h"
#include "ExpressionNode.h"

namespace escargot {

class ForInStatementNode : public StatementNode {
public:
    friend class ScriptParser;
    ForInStatementNode(Node *left, Node *right, Node *body, bool each)
        : StatementNode(NodeType::ForInStatement)
    {
        m_left = (ExpressionNode*) left;
        m_right = (ExpressionNode*) right;
        m_body = (StatementNode*) body;
        m_each = each;
    }

    virtual NodeType type() { return NodeType::ForInStatement; }

    virtual void generateStatementByteCode(CodeBlock* codeBlock, ByteCodeGenerateContext& context)
    {
        ByteCodeGenerateContext newContext(context);
        newContext.m_offsetToBasePointer = context.m_offsetToBasePointer + 1;

        m_right->generateExpressionByteCode(codeBlock, newContext);
        codeBlock->pushCode(DuplicateTopOfStackValue(), newContext, this);
        codeBlock->pushCode(Push(ESValue(ESValue::ESUndefined)), newContext, this);
        codeBlock->pushCode(Equal(), newContext, this);
        codeBlock->pushCode(JumpAndPopIfTopOfStackValueIsTrue(SIZE_MAX), newContext, this);
        size_t exit1Pos = codeBlock->lastCodePosition<JumpAndPopIfTopOfStackValueIsTrue>();

        codeBlock->pushCode(DuplicateTopOfStackValue(), newContext, this);
        codeBlock->pushCode(Push(ESValue(ESValue::ESNull)), newContext, this);
        codeBlock->pushCode(Equal(), newContext, this);
        codeBlock->pushCode(JumpAndPopIfTopOfStackValueIsTrue(SIZE_MAX), newContext, this);
        size_t exit2Pos = codeBlock->lastCodePosition<JumpAndPopIfTopOfStackValueIsTrue>();

        codeBlock->pushCode(EnumerateObject(), newContext, this);
#ifdef ENABLE_ESJIT
        codeBlock->pushCode(LoopStart(), newContext, this);
#endif
        size_t continuePosition = codeBlock->currentCodeSize();
        codeBlock->pushCode(CheckIfKeyIsLast(), newContext, this);
        codeBlock->pushCode(JumpAndPopIfTopOfStackValueIsTrue(SIZE_MAX), newContext, this);
        size_t exit3Pos = codeBlock->lastCodePosition<JumpIfTopOfStackValueIsTrue>();
        codeBlock->pushCode(EnumerateObjectKey(), newContext, this);

        size_t pushPos = codeBlock->currentCodeSize();
        codeBlock->pushCode(PushIntoTempStack(), newContext, this);
        m_left->generateResolveAddressByteCode(codeBlock, newContext);
        codeBlock->pushCode(PopFromTempStack(pushPos), newContext, this);
        m_left->generatePutByteCode(codeBlock, newContext);
        codeBlock->pushCode(Pop(), newContext, this);

        m_body->generateStatementByteCode(codeBlock, newContext);

        codeBlock->pushCode(Jump(continuePosition), newContext, this);
        size_t forInEnd = codeBlock->currentCodeSize();
        // codeBlock->pushCode(EnumerateObjectEnd(), this);
        codeBlock->pushCode(Pop(), newContext, this);
        ASSERT(codeBlock->peekCode<CheckIfKeyIsLast>(continuePosition)->m_orgOpcode == CheckIfKeyIsLastOpcode);

        newContext.consumeBreakPositions(codeBlock, forInEnd);
        newContext.consumeContinuePositions(codeBlock, continuePosition);
        newContext.m_positionToContinue = continuePosition;

        codeBlock->pushCode(Jump(SIZE_MAX), newContext, this);
        size_t jPos = codeBlock->lastCodePosition<Jump>();

        size_t exitPos = codeBlock->currentCodeSize();
        codeBlock->peekCode<JumpAndPopIfTopOfStackValueIsTrue>(exit1Pos)->m_jumpPosition = exitPos;
        codeBlock->peekCode<JumpAndPopIfTopOfStackValueIsTrue>(exit2Pos)->m_jumpPosition = exitPos;
        codeBlock->peekCode<JumpIfTopOfStackValueIsTrue>(exit3Pos)->m_jumpPosition = exitPos;

        codeBlock->peekCode<Jump>(jPos)->m_jumpPosition = codeBlock->currentCodeSize();

        newContext.propagateInformationTo(context);
    }

    virtual void computeRoughCodeBlockSizeInWordSize(size_t& result)
    {
        result += 30;
        m_right->computeRoughCodeBlockSizeInWordSize(result);
        m_left->computeRoughCodeBlockSizeInWordSize(result);
        m_body->computeRoughCodeBlockSizeInWordSize(result);
    }


protected:
    ExpressionNode *m_left;
    ExpressionNode *m_right;
    StatementNode *m_body;
    bool m_each;
};

}

#endif
