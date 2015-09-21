#ifndef SwitchStatementNode_h
#define SwitchStatementNode_h

#include "StatementNode.h"
#include "ExpressionNode.h"
#include "SwitchCaseNode.h"

namespace escargot {

class SwitchStatementNode : public StatementNode {
public:
    friend class ESScriptParser;
    SwitchStatementNode(Node* discriminant, StatementNodeVector&& casesA, Node* deflt, StatementNodeVector&& casesB, bool lexical)
            : StatementNode(NodeType::SwitchStatement)
    {
        m_discriminant = (ExpressionNode*) discriminant;
        m_casesA = casesA;
        m_default = (StatementNode*) deflt;
        m_casesB = casesB;
        m_lexical = lexical;
    }

    void executeStatement(ESVMInstance* instance)
    {
        /*
        ESValue input = m_discriminant->executeExpression(instance);
        instance->currentExecutionContext()->setJumpPositionAndExecute(true, [&](){
            bool found = false;
            unsigned i;
            for (i = 0; i < m_casesA.size(); i++) {
                SwitchCaseNode* caseNode = (SwitchCaseNode*) m_casesA[i];
                if (!found) {
                    ESValue clauseSelector = caseNode->m_test->executeExpression(instance);
                    if (input.equalsTo(clauseSelector)) {
                        found = true;
                    }
                }
                if (found)
                    caseNode->executeStatement(instance);
            }
            bool foundInB = false;
            if (!found) {
                for (i = 0; i < m_casesB.size(); i++) {
                    if (foundInB)
                        break;
                    SwitchCaseNode* caseNode = (SwitchCaseNode*) m_casesB[i];
                    ESValue clauseSelector = caseNode->m_test->executeExpression(instance);
                    if (input.equalsTo(clauseSelector)) {
                        foundInB = true;
                        caseNode->executeStatement(instance);
                    }
                }
            }
            if (!foundInB && m_default)
                m_default->executeStatement(instance);

            for (; i < m_casesB.size(); i++) {
                SwitchCaseNode* caseNode = (SwitchCaseNode*) m_casesB[i];
                caseNode->executeStatement(instance);
            }
        });
        */
    }

    virtual void generateStatementByteCode(CodeBlock* codeBlock, ByteCodeGenerateContext& context)
    {
        m_discriminant->generateExpressionByteCode(codeBlock, context);

        std::vector<size_t> jumpCodePerCaseNodePosition;
        for(unsigned i = 0; i < m_casesA.size() ; i ++) {
            SwitchCaseNode* caseNode = (SwitchCaseNode*) m_casesA[i];
            codeBlock->pushCode(DuplicateTopOfStackValue(), this);
            caseNode->m_test->generateExpressionByteCode(codeBlock, context);
            codeBlock->pushCode(Equal(), this);
            jumpCodePerCaseNodePosition.push_back(codeBlock->currentCodeSize());
            codeBlock->pushCode(JumpAndPopIfTopOfStackValueIsTrue(SIZE_MAX), this);
        }
        for(unsigned i = 0; i < m_casesB.size() ; i ++) {
            SwitchCaseNode* caseNode = (SwitchCaseNode*) m_casesB[i];
            codeBlock->pushCode(DuplicateTopOfStackValue(), this);
            caseNode->m_test->generateExpressionByteCode(codeBlock, context);
            codeBlock->pushCode(Equal(), this);
            jumpCodePerCaseNodePosition.push_back(codeBlock->currentCodeSize());
            codeBlock->pushCode(JumpAndPopIfTopOfStackValueIsTrue(SIZE_MAX), this);
        }
        size_t jmpToDefault = SIZE_MAX;
        codeBlock->pushCode(Pop(), this);
        jmpToDefault = codeBlock->currentCodeSize();
        codeBlock->pushCode(Jump(SIZE_MAX), this);

        size_t caseIdx = 0;
        for(unsigned i = 0; i < m_casesA.size() ; i ++) {
            SwitchCaseNode* caseNode = (SwitchCaseNode*) m_casesA[i];
            codeBlock->peekCode<JumpAndPopIfTopOfStackValueIsTrue>(jumpCodePerCaseNodePosition[caseIdx++])->m_jumpPosition = codeBlock->currentCodeSize();
            caseNode->generateStatementByteCode(codeBlock, context);
        }
        for(unsigned i = 0; i < m_casesB.size() ; i ++) {
            SwitchCaseNode* caseNode = (SwitchCaseNode*) m_casesB[i];
            codeBlock->peekCode<JumpAndPopIfTopOfStackValueIsTrue>(jumpCodePerCaseNodePosition[caseIdx++])->m_jumpPosition = codeBlock->currentCodeSize();
            caseNode->generateStatementByteCode(codeBlock, context);
        }
        if(m_default) {
            codeBlock->peekCode<Jump>(jmpToDefault)->m_jumpPosition = codeBlock->currentCodeSize();
            m_default->generateStatementByteCode(codeBlock, context);
        }
        size_t breakPos = codeBlock->currentCodeSize();
        context.consumeBreakPositions(codeBlock, breakPos);

        if(!m_default) {
            codeBlock->peekCode<Jump>(jmpToDefault)->m_jumpPosition = codeBlock->currentCodeSize();
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
