#include "Escargot.h"
#include "ESVMInstance.h"
#include "runtime/Environment.h"
#include "runtime/ExecutionContext.h"
#include "runtime/GlobalObject.h"
#include "parser/ESScriptParser.h"

namespace escargot {

ESVMInstance::ESVMInstance()
{
    std::setlocale(LC_ALL, "en_US.utf8");

    strings::initStaticStrings();

    m_globalObject = new GlobalObject();
    LexicalEnvironment* a = new LexicalEnvironment(new GlobalEnvironmentRecord(m_globalObject), NULL);

    m_globalExecutionContext = new ExecutionContext(a);
    m_currentExecutionContext = m_globalExecutionContext;

}

void ESVMInstance::evaluate(const std::string& source)
{
    try {
        Node* node = ESScriptParser::parseScript(source.c_str());
        node->execute(this);
    } catch(const char* e) {
        ESString str = e;
        wprintf(L"%ls\n", str.data());
    }


    /*
    //test/basic_ctx1.js
    ESValue* v = m_globalObject->get("a");
    ASSERT(v->toSmi()->value() == 1);
    */
}

}
