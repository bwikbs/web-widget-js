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

#ifndef ExecutionContext_h
#define ExecutionContext_h

#include "ESValue.h"
#include "bytecode/ByteCode.h"

namespace escargot {

class LexicalEnvironment;
class ExecutionContext : public gc {
public:
    ALWAYS_INLINE ExecutionContext(LexicalEnvironment* varEnv, bool isNewExpression, bool isStrictMode,
        ESValue* callStackInformation, ESValue* arguments = NULL, size_t argumentsCount = 0)
            : m_environment(varEnv)
            , m_callStackInformation((CallStackInformation*)callStackInformation)
            , m_tryOrCatchBodyResult(ESValue::ESForceUninitialized)
    {
        ASSERT(varEnv);
        m_data.m_isNewExpression = isNewExpression;
        m_data.m_isStrict = isStrictMode;
        m_argumentsInfo = arguments;
        m_data.m_argumentCount = argumentsCount;
#ifdef ENABLE_ESJIT
        m_inOSRExit = false;
        m_executeNextByteCode = false;
#endif
    }

    ALWAYS_INLINE LexicalEnvironment* environment()
    {
        // TODO
        return m_environment;
    }

    ALWAYS_INLINE void setEnvironment(LexicalEnvironment* env)
    {
        // TODO
        ASSERT(env);
        m_environment = env;
    }

    // http://www.ecma-international.org/ecma-262/6.0/index.html#sec-resolvebinding
    ESBindingSlot resolveBinding(const InternalAtomicString& atomicName);
    ESBindingSlot resolveBinding(const InternalAtomicString& atomicName, LexicalEnvironment*& env);

    ESValue* resolveArgumentsObjectBinding();

    // http://www.ecma-international.org/ecma-262/6.0/index.html#sec-resolvethisbinding
    void setThisBinding(const ESValue& v)
    {
        m_callStackInformation->m_thisValue = v;
    }

    void setCallee(const ESValue& callee)
    {
        m_callStackInformation->m_callee = callee;
    }

    ESValue& resolveThisBinding()
    {
        return m_callStackInformation->m_thisValue;
    }

    ESObject* resolveThisBindingToObject()
    {
        return m_callStackInformation->m_thisValue.toObject();
    }

    ESValue& resolveCallee()
    {
        return m_callStackInformation->m_callee;
    }

    // http://www.ecma-international.org/ecma-262/6.0/index.html#sec-getthisenvironment
    LexicalEnvironment* getThisEnvironment();

    ALWAYS_INLINE bool isNewExpression() { return m_data.m_isNewExpression; }

    // NOTE this argument information is for nativeFunctions. do not use this interpreter of jit
    ESValue* arguments() { return m_argumentsInfo; }
    size_t argumentCount() { return m_data.m_argumentCount; }
    ESValue readArgument(size_t idx)
    {
        if (idx < argumentCount()) {
            return arguments()[idx];
        } else {
            return ESValue();
        }
    }

    bool isStrictMode() { return m_data.m_isStrict; }
    void setStrictMode(bool strict) { m_data.m_isStrict = strict; };

#ifdef ENABLE_ESJIT
    bool inOSRExit() { return m_inOSRExit; }
    bool executeNextByteCode() { return m_executeNextByteCode; }
    char* getBp() { return m_stackBuf; }
    void setBp(char* bp) { m_stackBuf = bp; }
    unsigned getStackPos() { return m_stackPos; }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
    static size_t offsetOfArguments() { return offsetof(ExecutionContext, m_arguments); }
    static size_t offsetofInOSRExit() { return offsetof(ExecutionContext, m_inOSRExit); }
    static size_t offsetofExecuteNextByteCode() { return offsetof(ExecutionContext, m_executeNextByteCode); }
    static size_t offsetofStackBuf() { return offsetof(ExecutionContext, m_stackBuf); }
    static size_t offsetofStackPos() { return offsetof(ExecutionContext, m_stackPos); }
    static size_t offsetOfEnvironment() { return offsetof(ExecutionContext, m_environment); }
#pragma GCC diagnostic pop
#endif

    ESValue& tryOrCatchBodyReturnValue() { return m_tryOrCatchBodyResult; }
    ESValueVectorStd& tryOrCatchBodyResult() { return m_tryOrCatchBodyResultVector; }
private:
    struct {
        bool m_isNewExpression;
        bool m_isStrict;
        uint16_t m_argumentCount;
    } m_data;

    // TODO
    // LexicalEnvironment* m_lexicalEnvironment;
    // LexicalEnvironment* m_variableEnvironment;
    LexicalEnvironment* m_environment;

    struct CallStackInformation {
        ESValue m_thisValue;
        ESValue m_callee;
    };

    CallStackInformation* m_callStackInformation;

    union {
        ESValue m_tryOrCatchBodyResult;
        ESValue* m_argumentsInfo;
    };
    ESValueVectorStd m_tryOrCatchBodyResultVector;
#ifdef ENABLE_ESJIT
    bool m_inOSRExit;
    bool m_executeNextByteCode;
    unsigned m_stackPos;
    char* m_stackBuf;
#endif
};

}

#endif
