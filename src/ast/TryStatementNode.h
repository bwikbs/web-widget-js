#ifndef TryStatementNode_h
#define TryStatementNode_h

#include "StatementNode.h"
#include "runtime/ExecutionContext.h"
#include "CatchClauseNode.h"

namespace escargot {

class TryStatementNode : public StatementNode {
public:
    friend class ESScriptParser;
    TryStatementNode(Node *block, Node *handler, CatchClauseNodeVector&& guardedHandlers,  Node *finalizer)
        : StatementNode(NodeType::TryStatement)
      {
          m_block = (BlockStatementNode*) block;
          m_handler = (CatchClauseNode*) handler;
          m_guardedHandlers = guardedHandlers;
          m_finalizer = (BlockStatementNode*) finalizer;
      }


    void executeStatement(ESVMInstance* instance)
    {
        LexicalEnvironment* oldEnv = instance->currentExecutionContext()->environment();
        ExecutionContext* backupedEC = instance->currentExecutionContext();
        try {
            m_block->executeStatement(instance);
        } catch(const ESValue& err) {
            instance->invalidateIdentifierCacheCheckCount();
            instance->m_currentExecutionContext = backupedEC;
            LexicalEnvironment* catchEnv = new LexicalEnvironment(new DeclarativeEnvironmentRecord(), oldEnv);
            instance->currentExecutionContext()->setEnvironment(catchEnv);
            instance->currentExecutionContext()->environment()->record()->createMutableBinding(m_handler->param()->name(),
                    m_handler->param()->nonAtomicName());
            instance->currentExecutionContext()->environment()->record()->setMutableBinding(m_handler->param()->name(),
                    m_handler->param()->nonAtomicName()
                    , err, false);
            m_handler->executeStatement(instance);
            instance->currentExecutionContext()->setEnvironment(oldEnv);
        }
    }

protected:
    BlockStatementNode *m_block;
    CatchClauseNode *m_handler;
    CatchClauseNodeVector m_guardedHandlers;
    BlockStatementNode *m_finalizer;
};

}

#endif
