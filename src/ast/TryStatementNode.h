#ifndef TryStatementNode_h
#define TryStatementNode_h

#include "StatementNode.h"
#include "runtime/ExecutionContext.h"
#include "CatchClauseNode.h"

namespace escargot {

class TryStatementNode : public StatementNode {
public:
    friend class ScriptParser;
    TryStatementNode(Node *block, Node *handler, CatchClauseNodeVector&& guardedHandlers,  Node *finalizer)
        : StatementNode(NodeType::TryStatement)
    {
        m_block = (BlockStatementNode*) block;
        m_handler = (CatchClauseNode*) handler;
        m_guardedHandlers = guardedHandlers;
        m_finalizer = (BlockStatementNode*) finalizer;
    }

    virtual NodeType type() { return NodeType::TryStatement; }

    virtual void generateStatementByteCode(CodeBlock* codeBlock, ByteCodeGenerateContext& context)
    {
        context.m_tryStatementScopeCount++;
        codeBlock->pushCode(Try(), context, this);
        size_t pos = codeBlock->lastCodePosition<Try>();
        codeBlock->peekCode<Try>(pos)->m_tryDupCount = context.m_tryStatementScopeCount;
        m_block->generateStatementByteCode(codeBlock, context);

        codeBlock->pushCode(TryCatchBodyEnd(), context, this);
        size_t catchPos = codeBlock->currentCodeSize();
        if (m_handler) {
            m_handler->generateStatementByteCode(codeBlock, context);
        }
        codeBlock->pushCode(TryCatchBodyEnd(), context, this);

        context.registerJumpPositionsToComplexCase(pos);

        size_t endPos = codeBlock->currentCodeSize();
        if (m_handler) {
            codeBlock->peekCode<Try>(pos)->m_catchPosition = catchPos;
        } else {
            codeBlock->peekCode<Try>(pos)->m_catchPosition = 0;
        }
        codeBlock->peekCode<Try>(pos)->m_statementEndPosition = endPos;
        if (m_handler) {
            codeBlock->peekCode<Try>(pos)->m_name = m_handler->param()->name();
        } else {
            codeBlock->peekCode<Try>(pos)->m_name = strings->emptyString;
        }
        if (m_finalizer)
            m_finalizer->generateStatementByteCode(codeBlock, context);

        codeBlock->pushCode(FinallyEnd(), context, this);
        codeBlock->peekCode<FinallyEnd>(codeBlock->lastCodePosition<FinallyEnd>())->m_tryDupCount = context.m_tryStatementScopeCount;

        context.m_tryStatementScopeCount--;
    }

    virtual void computeRoughCodeBlockSizeInWordSize(size_t& result)
    {
        result += 4;
        m_block->computeRoughCodeBlockSizeInWordSize(result);
        if (m_handler)
            m_handler->computeRoughCodeBlockSizeInWordSize(result);
        if (m_finalizer)
            m_finalizer->computeRoughCodeBlockSizeInWordSize(result);
    }

protected:
    BlockStatementNode *m_block;
    CatchClauseNode *m_handler;
    CatchClauseNodeVector m_guardedHandlers;
    BlockStatementNode *m_finalizer;
};

}

#endif
