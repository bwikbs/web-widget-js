#ifndef LabeledStatementNode_h
#define LabeledStatementNode_h

#include "StatementNode.h"

namespace escargot {

class LabeledStatementNode : public StatementNode {
public:
    friend class ESScriptParser;
    LabeledStatementNode(StatementNode* statementNode, ESString* label)
            : StatementNode(NodeType::LabeledStatement)
    {
        m_statementNode = statementNode;
        m_label = label;
    }

    void executeStatement(ESVMInstance* instance)
    {
        instance->currentExecutionContext()->setLabelPositionAndExecute([this, &instance](){
            m_statementNode->executeStatement(instance);
        });

    }
protected:
    StatementNode* m_statementNode;
    ESString* m_label;
};

}

#endif