#ifndef NativeFunctionNode_h
#define NativeFunctionNode_h

#include "Node.h"

namespace escargot {

typedef std::function<ESValue* (ESVMInstance*)> NativeFunctionNodeFunctionType;

class NativeFunctionNode : public Node {
public:
    NativeFunctionNode(NativeFunctionNodeFunctionType&& fn)
            : Node(NodeType::NativeFunction)
    {
        m_nativeFunction = fn;
    }

    ESValue* execute(ESVMInstance* instance)
    {
        return m_nativeFunction(instance);
    }
protected:
    NativeFunctionNodeFunctionType m_nativeFunction;
};

}

#endif
