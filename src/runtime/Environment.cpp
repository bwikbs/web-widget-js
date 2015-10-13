#include "Escargot.h"
#include "Environment.h"
#include "GlobalObject.h"
#include "ESValue.h"
#include "ExecutionContext.h"
#include "ast/FunctionNode.h"
#include "vm/ESVMInstance.h"

namespace escargot {

//http://www.ecma-international.org/ecma-262/6.0/index.html#sec-newfunctionenvironment
//$8.1.2.4
LexicalEnvironment* LexicalEnvironment::newFunctionEnvironment(ESValue arguments[], const size_t& argumentCount, ESFunctionObject* function)
{
    FunctionEnvironmentRecord* envRec = new FunctionEnvironmentRecord(arguments, argumentCount, function->codeBlock()->m_innerIdentifiers);

    //envRec->m_functionObject = function;
    //envRec->m_newTarget = newTarget;

    LexicalEnvironment* env = new LexicalEnvironment(envRec, function->outerEnvironment());
    //TODO
    //If F’s [[ThisMode]] internal slot is lexical, set envRec.[[thisBindingStatus]] to "lexical".
    //[[ThisMode]] internal slot is lexical, set envRec.[[thisBindingStatus]] to "lexical".
    //Let home be the value of F’s [[HomeObject]] internal slot.
    //Set envRec.[[HomeObject]] to home.
    //Set envRec.[[NewTarget]] to newTarget.

    return env;
}

void EnvironmentRecord::createMutableBindingForAST(const InternalAtomicString& atomicName,ESString * name,bool canDelete)
{
    if(UNLIKELY(isGlobalEnvironmentRecord())) {
        toGlobalEnvironmentRecord()->createGlobalVarBinding(atomicName, name, canDelete);
        ESVMInstance::currentInstance()->invalidateIdentifierCacheCheckCount();
    } else {
        createMutableBinding(atomicName, name, canDelete);
    }
}

void DeclarativeEnvironmentRecord::createMutableBinding(const InternalAtomicString& name,ESString* nonAtomicName, bool canDelete)
{
    //TODO canDelete
    ASSERT(m_needsActivation);
    size_t siz = m_activationData.size();
    for(unsigned i = 0; i < siz ; i ++) {
        if(m_activationData[i].first == name) {
            return ;
        }
    }
    m_activationData.push_back(std::make_pair(name, ESValue()));
    ESVMInstance::currentInstance()->invalidateIdentifierCacheCheckCount();
}

//$8.1.1.4.12
bool GlobalEnvironmentRecord::hasVarDeclaration(const InternalAtomicString& name)
{
    if( std::find(m_varNames.begin(), m_varNames.end(), name) != m_varNames.end() )
        return true;
    return false;
}

//$8.1.1.4.15
bool GlobalEnvironmentRecord::canDeclareGlobalVar(const InternalAtomicString& name)
{
    RELEASE_ASSERT_NOT_REACHED();
    /*
    ESObject* globalObj = m_objectRecord->bindingObject();
    bool hasProperty = globalObj->hasOwnProperty(name);
    if (hasProperty)
        return true;
    else
        return globalObj->isExtensible();
        */
}

//$8.1.1.4.16
bool GlobalEnvironmentRecord::canDeclareGlobalFunction(const InternalAtomicString& name)
{
    RELEASE_ASSERT_NOT_REACHED();
    /*
    ESObject* globalObj = m_objectRecord->bindingObject();
    ESSlot* pd = globalObj->find(name);
    if(pd == NULL)
        return globalObj->isExtensible();

    if (pd->isConfigurable() == true)
        return true;

    // IsDataDescriptor && ..
     */
    
    return false;
}

//$8.1.1.4.17
void GlobalEnvironmentRecord::createGlobalVarBinding(const InternalAtomicString& name,ESString * nonAtomicName, bool canDelete)
{
    ESObject* globalObj = m_objectRecord->bindingObject();
    bool hasProperty = globalObj->hasOwnProperty(nonAtomicName);
    bool extensible = globalObj->isExtensible();
    if (!hasProperty && extensible) {
        m_objectRecord->createMutableBinding(name, nonAtomicName, canDelete);
        m_objectRecord->initializeBinding(name, nonAtomicName, ESValue());
    }
    if( std::find(m_varNames.begin(), m_varNames.end(), name) == m_varNames.end() )
        m_varNames.push_back(name);
}

//$8.1.1.4.18
void GlobalEnvironmentRecord::createGlobalFunctionBinding(const InternalAtomicString& name,ESString * nonAtomicName, const ESValue& V, bool canDelete) {
    ESObject* globalObj = m_objectRecord->bindingObject();
    globalObj->defineDataProperty(nonAtomicName, true, true, canDelete, V);
    if( std::find(m_varNames.begin(), m_varNames.end(), name) == m_varNames.end() )
        m_varNames.push_back(name);
}

//$8.1.1.4.11
ESValue GlobalEnvironmentRecord::getThisBinding() {
    return m_objectRecord->bindingObject();
}

//$8.1.1.4.1
ESValue* GlobalEnvironmentRecord::hasBinding(const InternalAtomicString& atomicName, ESString * name) {
    ESValue* ret = m_declarativeRecord->hasBinding(atomicName, name);
    if(ret)
        return ret;
    return m_objectRecord->hasBinding(atomicName, name);
}

//$8.1.1.4.2
void GlobalEnvironmentRecord::createMutableBinding(const InternalAtomicString& name,ESString * nonAtomicName, bool canDelete) {
    if( m_declarativeRecord->hasBinding(name, nonAtomicName) )
        throw "TypeError";
    m_declarativeRecord->createMutableBinding(name, nonAtomicName, canDelete);
}

//$8.1.1.4.4
void GlobalEnvironmentRecord::initializeBinding(const InternalAtomicString& name,ESString * nonAtomicName, const ESValue& V) {
    if( m_declarativeRecord->hasBinding(name, nonAtomicName))
        m_declarativeRecord->initializeBinding(name, nonAtomicName, V);
    else {
        ASSERT(m_objectRecord->hasBinding(name, nonAtomicName));
        m_objectRecord->initializeBinding(name, nonAtomicName, V);
    }
    m_objectRecord->initializeBinding(name, nonAtomicName, V);
}

//$8.1.1.4.6
/*
ESValue GlobalEnvironmentRecord::getBindingValue(const InternalAtomicString& name, bool ignoreReferenceErrorException) {
    //FIXME
    //if( m_declarativeRecord->hasBinding(name) )
    //    return m_declarativeRecord->getBindingValue(name, ignoreReferenceErrorException);
    //else {
    //    return m_objectRecord->getBindingValue(name, ignoreReferenceErrorException);
    //}
    return m_objectRecord->getBindingValue(name, ignoreReferenceErrorException);
}*/

//$8.1.1.4.5
//http://www.ecma-international.org/ecma-262/6.0/index.html#sec-global-environment-records-setmutablebinding-n-v-s
void GlobalEnvironmentRecord::setMutableBinding(const InternalAtomicString& name, ESString * nonAtomicName, const ESValue& V, bool S) {
    if( m_declarativeRecord->hasBinding(name, nonAtomicName)) {
        m_declarativeRecord->setMutableBinding(name, nonAtomicName, V, S);
    } else {
        m_objectRecord->setMutableBinding(name, nonAtomicName, V, S);
    }
}

//$8.1.1.2.2
void ObjectEnvironmentRecord::createMutableBinding(const InternalAtomicString& name,ESString * nonAtomicName, bool canDelete)
{
    m_bindingObject->defineDataProperty(nonAtomicName, true, true, canDelete);
}

//$8.1.1.2.4
void ObjectEnvironmentRecord::initializeBinding(const InternalAtomicString& name,ESString * nonAtomicName, const ESValue& V)
{
    return setMutableBinding(name, nonAtomicName, V, false);
}

//$8.1.1.2.5
void ObjectEnvironmentRecord::setMutableBinding(const InternalAtomicString& name,ESString * nonAtomicName, const ESValue& V, bool S)
{
    m_bindingObject->set(nonAtomicName, V, S);
}

//http://www.ecma-international.org/ecma-262/6.0/index.html#sec-bindthisvalue
void FunctionEnvironmentRecord::bindThisValue(const ESValue& V)
{
#ifndef NDEBUG
    ASSERT(m_thisBindingStatus != Initialized);
    if(m_thisBindingStatus == Lexical)
        throw ReferenceError::create();
    m_thisBindingStatus = Initialized;
#endif
    m_thisValue = V;
}

ESValue FunctionEnvironmentRecord::getThisBinding()
{
#ifndef NDEBUG
    ASSERT(m_thisBindingStatus != Lexical);
    if(m_thisBindingStatus == Uninitialized)
        throw ReferenceError::create();
#endif
    return m_thisValue;
}

}
