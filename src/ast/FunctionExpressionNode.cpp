#include "Escargot.h"
#include "FunctionExpressionNode.h"
#include "vm/ESVMInstance.h"
#include "runtime/ExecutionContext.h"
#include "runtime/Environment.h"

namespace escargot {

ESValue* FunctionExpressionNode::execute(ESVMInstance* instance)
{
    JSFunction* function = JSFunction::create(instance->currentExecutionContext()->environment(), this);
    //FIXME these lines duplicate with FunctionDeclarationNode::execute
    function->set__proto__(instance->globalObject()->functionPrototype());
    JSObject* prototype = JSObject::create();
    prototype->setConstructor(function);
    prototype->set__proto__(instance->globalObject()->object());
    function->setProtoType(prototype);
    function->set(strings->name, JSString::create(m_id.data()));
    /////////////////////////////////////////////////////////////////////

    return function;
}

}
