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

#ifndef SwitchStatementNode_h
#define SwitchStatementNode_h

#include "StatementNode.h"
#include "ExpressionNode.h"
#include "SwitchCaseNode.h"

namespace escargot {

class SwitchStatementNode : public StatementNode {
public:
    friend class ScriptParser;
    SwitchStatementNode(Node* discriminant, StatementNodeVector&& casesA, Node* deflt, StatementNodeVector&& casesB, bool lexical)
        : StatementNode(NodeType::SwitchStatement)
    {
        m_discriminant = (ExpressionNode*) discriminant;
        m_casesA = casesA;
        m_default = (StatementNode*) deflt;
        m_casesB = casesB;
        m_lexical = lexical;
    }

    virtual NodeType type() { return NodeType::SwitchStatement; }

    virtual void generateStatementByteCode(CodeBlock* codeBlock, ByteCodeGenerateContext& context)
    {
        ByteCodeGenerateContext newContext(context);
        m_discriminant->generateExpressionByteCode(codeBlock, newContext);

        std::vector<size_t> jumpCodePerCaseNodePosition;
        for (unsigned i = 0; i < m_casesB.size() ; i ++) {
            SwitchCaseNode* caseNode = (SwitchCaseNode*) m_casesB[i];
            codeBlock->pushCode(DuplicateTopOfStackValue(), newContext, this);
            caseNode->m_test->generateExpressionByteCode(codeBlock, newContext);
            codeBlock->pushCode(StrictEqual(), newContext, this);
            jumpCodePerCaseNodePosition.push_back(codeBlock->currentCodeSize());
            codeBlock->pushCode(JumpAndPopIfTopOfStackValueIsTrue(SIZE_MAX), newContext, this);
        }
        for (unsigned i = 0; i < m_casesA.size() ; i ++) {
            SwitchCaseNode* caseNode = (SwitchCaseNode*) m_casesA[i];
            codeBlock->pushCode(DuplicateTopOfStackValue(), newContext, this);
            caseNode->m_test->generateExpressionByteCode(codeBlock, newContext);
            codeBlock->pushCode(StrictEqual(), newContext, this);
            jumpCodePerCaseNodePosition.push_back(codeBlock->currentCodeSize());
            codeBlock->pushCode(JumpAndPopIfTopOfStackValueIsTrue(SIZE_MAX), newContext, this);
        }
        size_t jmpToDefault = SIZE_MAX;
        codeBlock->pushCode(Pop(), newContext, this);
        jmpToDefault = codeBlock->currentCodeSize();
        codeBlock->pushCode(Jump(SIZE_MAX), newContext, this);

        size_t caseIdx = 0;
        for (unsigned i = 0; i < m_casesB.size() ; i ++) {
            SwitchCaseNode* caseNode = (SwitchCaseNode*) m_casesB[i];
            codeBlock->peekCode<JumpAndPopIfTopOfStackValueIsTrue>(jumpCodePerCaseNodePosition[caseIdx++])->m_jumpPosition = codeBlock->currentCodeSize();
            caseNode->generateStatementByteCode(codeBlock, newContext);
        }
        if (m_default) {
            codeBlock->peekCode<Jump>(jmpToDefault)->m_jumpPosition = codeBlock->currentCodeSize();
            m_default->generateStatementByteCode(codeBlock, newContext);
        }
        for (unsigned i = 0; i < m_casesA.size() ; i ++) {
            SwitchCaseNode* caseNode = (SwitchCaseNode*) m_casesA[i];
            codeBlock->peekCode<JumpAndPopIfTopOfStackValueIsTrue>(jumpCodePerCaseNodePosition[caseIdx++])->m_jumpPosition = codeBlock->currentCodeSize();
            caseNode->generateStatementByteCode(codeBlock, newContext);
        }
        size_t breakPos = codeBlock->currentCodeSize();
        newContext.consumeBreakPositions(codeBlock, breakPos);
        newContext.m_positionToContinue = context.m_positionToContinue;
        newContext.propagateInformationTo(context);

        if (!m_default) {
            codeBlock->peekCode<Jump>(jmpToDefault)->m_jumpPosition = codeBlock->currentCodeSize();
        }
    }

    virtual void computeRoughCodeBlockSizeInWordSize(size_t& result)
    {
        result += 30;
        if (m_discriminant)
            m_discriminant->computeRoughCodeBlockSizeInWordSize(result);
        if (m_default)
            m_default->computeRoughCodeBlockSizeInWordSize(result);
        for (unsigned i = 0; i < m_casesA.size() ; i ++) {
            m_casesA[i]->computeRoughCodeBlockSizeInWordSize(result);
        }
        for (unsigned i = 0; i < m_casesB.size() ; i ++) {
            m_casesB[i]->computeRoughCodeBlockSizeInWordSize(result);
        }
    }

protected:
    ExpressionNode* m_discriminant;
    StatementNodeVector m_casesA;
    StatementNode* m_default;
    StatementNodeVector m_casesB;
    bool m_lexical;
};

}

#endif
