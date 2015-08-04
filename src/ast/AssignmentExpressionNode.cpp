#include "Escargot.h"
#include "AssignmentExpressionNode.h"

#include "IdentifierNode.h"
#include "vm/ESVMInstance.h"
#include "runtime/ExecutionContext.h"
#include "runtime/Environment.h"

namespace escargot {

ESValue AssignmentExpressionNode::execute(ESVMInstance* instance)
{
    ESValue rvalue;
    switch(m_operator) {
    case SimpleAssignment:
    {
        //http://www.ecma-international.org/ecma-262/5.1/#sec-11.13.1
        rvalue = m_right->execute(instance);
        writeValue(instance, m_left, rvalue);
        break;
    }
    case CompoundAssignment:
    {
        rvalue = BinaryExpressionNode::execute(instance, m_left->execute(instance), m_right->execute(instance), m_compoundOperator);
        writeValue(instance, m_left, rvalue);
        break;
    }
    default:
        RELEASE_ASSERT_NOT_REACHED();
        break;
    }

    return rvalue;
}

void AssignmentExpressionNode::writeValue(ESVMInstance* instance, Node* leftHandNode, const ESValue& rvalue)
{
    if(leftHandNode->type() == NodeType::Identifier) {
        IdentifierNode* idNode = (IdentifierNode *)leftHandNode;
        try {
            idNode->executeForWrite(instance)->setValue(rvalue);
        } catch(ESValue& err) {
            if(err.isESPointer() && err.asESPointer()->isESObject() &&
                    (err.asESPointer()->asESObject()->constructor().asESPointer() == instance->globalObject()->referenceError())) {
                //TODO set proper flags
                instance->globalObject()->set(idNode->name(), rvalue);
            } else {
                throw err;
            }
        }

    } else {
        instance->currentExecutionContext()->resetLastESObjectMetInMemberExpressionNode();
        leftHandNode->execute(instance);
        ESObject* obj = instance->currentExecutionContext()->lastESObjectMetInMemberExpressionNode();
        if(UNLIKELY(!obj)) {
            throw ESValue(ESString::create(L"could not assign to left hand node lastESObjectMetInMemberExpressionNode==NULL"));
        }
        if(obj->isESArrayObject()) {
            obj->asESArrayObject()->set(instance->currentExecutionContext()->lastUsedPropertyValueInMemberExpressionNode(), rvalue);
        } else {
            obj->set(instance->currentExecutionContext()->lastUsedPropertyNameInMemberExpressionNode(), rvalue);
        }
    }
}

}

