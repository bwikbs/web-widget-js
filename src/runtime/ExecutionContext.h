#ifndef ExecutionContext_h
#define ExecutionContext_h

#include "ESValue.h"

namespace escargot {

struct jmpbuf_wrapper {
    std::jmp_buf m_buffer;
};

class LexicalEnvironment;
class ExecutionContext : public gc {
public:
    ExecutionContext(LexicalEnvironment* varEnv, bool needsActivation, bool isNewExpression, ESValue* arguments = NULL, size_t argumentsCount = 0);
    ALWAYS_INLINE LexicalEnvironment* environment()
    {
        //TODO
        return m_variableEnvironment;
    }

    ALWAYS_INLINE void setEnvironment(LexicalEnvironment* env)
    {
        //TODO
        m_variableEnvironment = env;
    }

    //http://www.ecma-international.org/ecma-262/6.0/index.html#sec-resolvebinding
    ESSlot* resolveBinding(const InternalAtomicString& name);

    //http://www.ecma-international.org/ecma-262/6.0/index.html#sec-resolvethisbinding
    ESObject* resolveThisBinding();

    //http://www.ecma-international.org/ecma-262/6.0/index.html#sec-getthisenvironment
    LexicalEnvironment* getThisEnvironment();

    ALWAYS_INLINE void resetLastESObjectMetInMemberExpressionNode()
    {
        m_lastESObjectMetInMemberExpressionNode = NULL;
    }

    ALWAYS_INLINE ESObject* lastESObjectMetInMemberExpressionNode()
    {
        return m_lastESObjectMetInMemberExpressionNode;
    }

    ALWAYS_INLINE const InternalAtomicString& lastUsedPropertyNameInMemberExpressionNode()
    {
        return m_lastUsedPropertyNameInMemberExpressionNode;
    }

    ALWAYS_INLINE ESValue lastUsedPropertyValueInMemberExpressionNode()
    {
        return m_lastUsedPropertyValueInMemberExpressionNode;
    }

    ALWAYS_INLINE void setLastESObjectMetInMemberExpressionNode(ESObject* obj, const InternalAtomicString& name, const ESValue& value)
    {
        m_lastESObjectMetInMemberExpressionNode = obj;
        m_lastUsedPropertyNameInMemberExpressionNode = name;
        m_lastUsedPropertyValueInMemberExpressionNode = value;
    }

    void doReturn(const ESValue& returnValue)
    {
        m_returnValue = returnValue;
        std::longjmp(m_returnPosition,1);
    }

    std::jmp_buf& returnPosition() { return m_returnPosition; }

    void doBreak()
    {
        longjmp(m_breakPositions.back().m_buffer, 1);
    }

    void doContinue()
    {
        longjmp(m_continuePositions.back().m_buffer, 1);
    }

    void pushContinuePosition(jmpbuf_wrapper& pos)
    {
        m_continuePositions.push_back(pos);
    }

    void popContinuePosition()
    {
        m_continuePositions.pop_back();
    }

    template <typename T>
    void setJumpPositionAndExecute(const T& fn) {
        jmpbuf_wrapper newone;
        int r = setjmp(newone.m_buffer);
        if (r != 1) {
            m_breakPositions.push_back(newone);
            fn();
        }
        m_breakPositions.pop_back();
    }

    ESValue returnValue()
    {
        return m_returnValue;
    }

    ALWAYS_INLINE bool needsActivation() { return m_needsActivation; } //child & parent AST has eval, with, catch
    ALWAYS_INLINE bool isNewExpression() { return m_isNewExpression; }
    ESValue* arguments() { return m_arguments; }
    size_t argumentCount() { return m_argumentCount; }

private:
    ESFunctionObject* m_function;
    bool m_needsActivation;
    bool m_isNewExpression;
    ESValue* m_arguments;
    size_t m_argumentCount;
    LexicalEnvironment* m_lexicalEnvironment;
    LexicalEnvironment* m_variableEnvironment;
    ESObject* m_lastESObjectMetInMemberExpressionNode;
    InternalAtomicString m_lastUsedPropertyNameInMemberExpressionNode;
    ESValue m_lastUsedPropertyValueInMemberExpressionNode;
    ESValue m_returnValue;
    std::jmp_buf m_returnPosition;
    std::vector<jmpbuf_wrapper> m_breakPositions;
    std::vector<jmpbuf_wrapper> m_continuePositions;
};

}

#endif
