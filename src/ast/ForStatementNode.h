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

#ifndef ForStatementNode_h
#define ForStatementNode_h

#include "StatementNode.h"
#include "ExpressionNode.h"

namespace escargot {

class ForStatementNode : public StatementNode {
public:
    friend class ScriptParser;
    ForStatementNode(Node *init, Node *test, Node *update, Node *body)
        : StatementNode(NodeType::ForStatement)
    {
        m_init = (ExpressionNode*) init;
        m_test = (ExpressionNode*) test;
        m_update = (ExpressionNode*) update;
        m_body = (StatementNode*) body;
    }

    virtual NodeType type() { return NodeType::ForStatement; }

    virtual void generateStatementByteCode(CodeBlock* codeBlock, ByteCodeGenerateContext& context)
    {
        ByteCodeGenerateContext newContext(context);

        if (m_init) {
            size_t start = codeBlock->currentCodeSize();
            m_init->generateExpressionByteCode(codeBlock, newContext);
            size_t end = codeBlock->currentCodeSize();
            if (start != end)
                codeBlock->pushCode(Pop(), newContext, this);
        }

#ifdef ENABLE_ESJIT
        codeBlock->pushCode(LoopStart(), newContext, this);
#endif

        size_t forStart = codeBlock->currentCodeSize();

        if (m_test) {
            m_test->generateExpressionByteCode(codeBlock, newContext);
        } else {
            codeBlock->pushCode(Push(ESValue(true)), newContext, this);
        }

        codeBlock->pushCode(JumpIfTopOfStackValueIsFalse(SIZE_MAX), newContext, this);
        size_t testPos = codeBlock->lastCodePosition<JumpIfTopOfStackValueIsFalse>();

        m_body->generateStatementByteCode(codeBlock, newContext);

        size_t updatePosition = codeBlock->currentCodeSize();
        if (m_update) {
            m_update->generateExpressionByteCode(codeBlock, newContext);
            codeBlock->pushCode(Pop(), newContext, this);
        }
        codeBlock->pushCode(Jump(forStart), newContext, this);

        size_t forEnd = codeBlock->currentCodeSize();
        codeBlock->peekCode<JumpIfTopOfStackValueIsFalse>(testPos)->m_jumpPosition = forEnd;

        newContext.consumeBreakPositions(codeBlock, forEnd);
        newContext.consumeContinuePositions(codeBlock, updatePosition);
        newContext.m_positionToContinue = updatePosition;
        newContext.propagateInformationTo(context);
    }

    virtual void computeRoughCodeBlockSizeInWordSize(size_t& result)
    {
        result += 10;
        if (m_init)
            m_init->computeRoughCodeBlockSizeInWordSize(result);
        if (m_test)
            m_test->computeRoughCodeBlockSizeInWordSize(result);
        if (m_update)
            m_update->computeRoughCodeBlockSizeInWordSize(result);
        m_body->computeRoughCodeBlockSizeInWordSize(result);
    }

protected:
    ExpressionNode *m_init;
    ExpressionNode *m_test;
    ExpressionNode *m_update;
    StatementNode *m_body;
};

}

#endif
