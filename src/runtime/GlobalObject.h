#ifndef GlobalObject_h
#define GlobalObject_h

#include "ESValue.h"

namespace escargot {

class JSBuiltinsObject;
class GlobalObject : public JSObject {
public:
    friend class ESVMInstance;
    GlobalObject();

    ALWAYS_INLINE escargot::JSFunction* object()
    {
        return m_object;
    }

    ALWAYS_INLINE JSObject* objectPrototype()
    {
        return m_objectPrototype;
    }

    ALWAYS_INLINE escargot::JSFunction* function()
    {
        return m_function;
    }

    ALWAYS_INLINE escargot::JSFunction* functionPrototype()
    {
        return m_functionPrototype;
    }

    ALWAYS_INLINE escargot::JSFunction* array()
    {
        return m_array;
    }

    ALWAYS_INLINE JSObject* arrayPrototype()
    {
        return m_arrayPrototype;
    }
protected:
    void installObject();
    void installFunction();
    void installArray();
    escargot::JSFunction* m_object;
    escargot::JSObject* m_objectPrototype;
    escargot::JSFunction* m_function;
    escargot::JSFunction* m_functionPrototype;
    escargot::JSFunction* m_array;
    escargot::JSObject* m_arrayPrototype;
    //JSBuiltinsObject* m_builtins;
    //Context* m_nativeContext;
};

class JSGlobalObject : public GlobalObject {
};

class JSBuiltinsObject : public GlobalObject {
};

}

#endif
