#ifndef FunctionExpressionNode_h
#define FunctionExpressionNode_h

#include "FunctionNode.h"

namespace escargot {

class FunctionExpressionNode : public FunctionNode {
public:
    friend class ScriptParser;
    FunctionExpressionNode(const InternalAtomicString& id, InternalAtomicStringVector&& params, Node* body, bool isGenerator, bool isExpression, bool isStrict)
        : FunctionNode(NodeType::FunctionExpression, id, std::move(params), body, isGenerator, isExpression, isStrict)
    {
        m_isGenerator = false;
        m_isExpression = true;
    }

    virtual NodeType type() { return NodeType::FunctionExpression; }

    virtual void generateExpressionByteCode(CodeBlock* codeBlock, ByteCodeGenerateContext& context)
    {
        // size_t myResult = 0;
        // m_body->computeRoughCodeBlockSizeInWordSize(myResult);

        // CodeBlock* cb = CodeBlock::create(myResult);
        CodeBlock* cb = CodeBlock::create(0);
        if (context.m_shouldGenereateByteCodeInstantly) {
            cb->m_innerIdentifiersSize = m_innerIdentifiers.size();
            if (m_needsHeapAllocatedVariableStorage)
                cb->m_innerIdentifiers = std::move(m_innerIdentifiers);
            cb->m_needsToPrepareGenerateArgumentsObject = m_needsToPrepareGenerateArgumentsObject;
            cb->m_needsHeapAllocatedVariableStorage = m_needsHeapAllocatedVariableStorage;
            cb->m_needsActivation = m_needsActivation;
            cb->m_params = std::move(m_params);
            cb->m_isStrict = m_isStrict;
            cb->m_isFunctionExpression = true;

            ByteCodeGenerateContext newContext(cb);
            m_body->generateStatementByteCode(cb, newContext);
            cb->pushCode(ReturnFunction(), newContext, this);
#ifndef NDEBUG
            cb->m_nonAtomicId = m_nonAtomicId;
            if (ESVMInstance::currentInstance()->m_reportUnsupportedOpcode) {
                char* code = cb->m_code.data();
                ByteCode* currentCode = (ByteCode *)(&code[0]);
                if (currentCode->m_orgOpcode != ExecuteNativeFunctionOpcode) {
                    dumpUnsupported(cb);
                }
            }
#endif
            codeBlock->pushCode(CreateFunction(m_id, m_nonAtomicId, cb, false, m_functionIdIndex), context, this);
        } else {
            cb->m_ast = this;
            cb->m_params = m_params;
            codeBlock->pushCode(CreateFunction(m_id, m_nonAtomicId, cb, false, m_functionIdIndex), context, this);
        }
    }

    virtual void computeRoughCodeBlockSizeInWordSize(size_t& result)
    {
        result += 6;
    }


protected:
    ExpressionNodeVector m_defaults; // defaults: [ Expression ];
    // rest: Identifier | null;
};

}

#endif
