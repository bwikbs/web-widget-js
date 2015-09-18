#ifndef IdentifierNode_h
#define IdentifierNode_h

#include "Node.h"
#include "ExpressionNode.h"
#include "PatternNode.h"

namespace escargot {

//interface Identifier <: Node, Expression, Pattern {
class IdentifierNode : public Node {
public:
    friend class ESScriptParser;
    IdentifierNode(const InternalAtomicString& name)
            : Node(NodeType::Identifier)
    {
        m_name = name;
        m_nonAtomicName = ESString::create(name.data());
        m_identifierCacheInvalidationCheckCount = std::numeric_limits<unsigned>::max();
        m_canUseFastAccess = false;
        m_fastAccessIndex = SIZE_MAX;
        m_fastAccessUpIndex = SIZE_MAX;
    }

    virtual void generateExpressionByteCode(CodeBlock* codeBlock, ByteCodeGenerateContext& context)
    {
        if(m_canUseFastAccess) {
            if(codeBlock->m_needsActivation) {
                codeBlock->pushCode(GetByIndexWithActivation(m_fastAccessIndex, m_fastAccessUpIndex), this);
#ifndef NDEBUG
                codeBlock->peekCode<GetByIndexWithActivation>(codeBlock->lastCodePosition<GetByIndexWithActivation>())->m_name = m_nonAtomicName;
#endif
            } else {
                if(m_fastAccessUpIndex == 0) {
                    codeBlock->pushCode(GetByIndex(m_fastAccessIndex), this);
#ifndef NDEBUG
                    codeBlock->peekCode<GetByIndex>(codeBlock->lastCodePosition<GetByIndex>())->m_name = m_nonAtomicName;
#endif
                } else {
                    codeBlock->pushCode(GetByIndexWithActivation(m_fastAccessIndex, m_fastAccessUpIndex), this);
#ifndef NDEBUG
                    codeBlock->peekCode<GetByIndexWithActivation>(codeBlock->lastCodePosition<GetByIndexWithActivation>())->m_name = m_nonAtomicName;
#endif
                }
            }
        } else {
            codeBlock->pushCode(GetById(m_name, m_nonAtomicName), this);
        }
    }

    virtual void generateResolveAddressByteCode(CodeBlock* codeBlock, ByteCodeGenerateContext& context)
    {
    }

    virtual void generateReferenceResolvedAddressByteCode(CodeBlock* codeBlock, ByteCodeGenerateContext& context)
    {
        generateExpressionByteCode(codeBlock, context);
    }


    virtual void generatePutByteCode(CodeBlock* codeBlock, ByteCodeGenerateContext& context)
    {
        if(m_canUseFastAccess) {
            if(codeBlock->m_needsActivation) {
                codeBlock->pushCode(PutByIndexWithActivation(m_fastAccessIndex, m_fastAccessUpIndex), this);
            } else {
                if(m_fastAccessUpIndex == 0)
                    codeBlock->pushCode(PutByIndex(m_fastAccessIndex), this);
                else
                    codeBlock->pushCode(PutByIndexWithActivation(m_fastAccessIndex, m_fastAccessUpIndex), this);
            }
        } else {
            codeBlock->pushCode(PutById(m_name, m_nonAtomicName), this);
        }
    }

    const InternalAtomicString& name()
    {
        return m_name;
    }

    ESString* nonAtomicName()
    {
        return m_nonAtomicName;
    }

    void setFastAccessIndex(size_t upIndex, size_t index)
    {
        m_canUseFastAccess = true;
        m_fastAccessIndex = index;
        m_fastAccessUpIndex = upIndex;
    }

    bool canUseFastAccess()
    {
        return m_canUseFastAccess;
    }

    size_t fastAccessIndex()
    {
        return m_fastAccessIndex;
    }

    size_t fastAccessUpIndex()
    {
        return m_fastAccessUpIndex;
    }

protected:
    InternalAtomicString m_name;
    ESString* m_nonAtomicName;

    unsigned m_identifierCacheInvalidationCheckCount;
    ESSlotAccessor m_cachedSlot;

    bool m_canUseFastAccess;
    size_t m_fastAccessIndex;
    size_t m_fastAccessUpIndex;
};

}

#endif
