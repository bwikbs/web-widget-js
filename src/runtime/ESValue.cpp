#include "Escargot.h"
#include "ESValue.h"

#include "vm/ESVMInstance.h"
#include "runtime/ExecutionContext.h"
#include "runtime/Environment.h"
#include "ast/AST.h"

namespace escargot {

/*
static ESNumber s_nan(std::numeric_limits<double>::quiet_NaN());
ESNumber* esNaN = &s_nan;

static ESNumber s_infinity(std::numeric_limits<double>::infinity());
ESNumber* esInfinity = &s_infinity;
static ESNumber s_ninfinity(-std::numeric_limits<double>::infinity());
ESNumber* esNegInfinity = &s_ninfinity;

static ESNumber s_nzero(-0.0);
ESNumber* esMinusZero = &s_nzero;
*/

// http://www.ecma-international.org/ecma-262/6.0/index.html#sec-abstract-equality-comparison
bool ESValue::abstractEqualsTo(const ESValue& val)
{
    if (isInt32() && val.isInt32()) {
        return asInt32() == val.asInt32();
    } else if (isNumber() && val.isNumber()) {
        double a = asNumber();
        double b = val.asNumber();
        if (std::isnan(a) || std::isnan(b)) return false;
        else if (a == b) return true;
        else return false;
    } else if (isUndefined() && val.isUndefined()) {
        return true;
    } else if (isNull() && val.isNull()) {
        return true;
    } else if (isBoolean() && val.isBoolean()) {
        return asBoolean() == val.asBoolean();
    } else if (isESPointer() && val.isESPointer()) {
        ESPointer* o = asESPointer();
        ESPointer* comp = val.asESPointer();
        if (o->type() == comp->type())
            return equalsTo(val);
    } else {
        if (isNull() && val.isUndefined()) return true;
        else if (isUndefined() && val.isNull()) return true;
        else if (isNumber() && (val.isESString() || val.isBoolean())) {
            return asNumber() == val.toNumber();
        }
        else if ((isESString() || isBoolean()) && val.isNumber()) {
            return val.asNumber() == toNumber();
        }
        else if ((isESString() || isNumber()) && val.isObject()) {
            return abstractEqualsTo(val.toPrimitive());
        }
        else if (isObject() && (val.isESString() || val.isNumber())) {
            return toPrimitive().abstractEqualsTo(val);
        }
    }
    return false;
}

bool ESValue::equalsTo(const ESValue& val)
{
    if(isNumber()) {
        return asNumber() == val.toNumber();
    }
    if(isBoolean())
        return asBoolean() == val.toBoolean();

    if(isESPointer()) {
        ESPointer* o = asESPointer();
        if (!val.isESPointer())
            return false;
        ESPointer* o2 = val.asESPointer();
        if (o->type() != o2->type())
            return false;
        //Strict Equality Comparison: ===
        if(o->isESString() && o->asESString()->string() == o2->asESString()->string())
            return true;
        if(o->isESStringObject() && o->asESStringObject()->getStringData()->string() == o2->asESStringObject()->getStringData()->string())
            return true;
        //TODO
        return false;
    }
    RELEASE_ASSERT_NOT_REACHED();
}


ESString* ESValue::toString() const
{
    if(isPrimitive()) {
        InternalStringStd ret;
        if(isInt32()) {
            ret = std::move(std::to_wstring(asInt32()));
        } else if(isNumber()) {
            double d = asNumber();
            if (std::isnan(d)) ret = L"NaN";
            else if (d == std::numeric_limits<double>::infinity()) ret = L"Infinity";
            else if (d== -std::numeric_limits<double>::infinity()) ret = L"-Infinity";
            else {
                wchar_t buf[512];
                char chbuf[50];
                char* end = rapidjson::internal::dtoa(toNumber(), chbuf);
                int i = 0;
                for (char* p = chbuf; p != end; ++p)
                    buf[i++] = (wchar_t) *p;
                buf[i] = L'\0';
                //std::swprintf(buf, 511, L"%.17lg", number);
                size_t tl = wcslen(buf);
                ret.reserve(tl);
                ret.append(buf);
            }
        } else if(isUndefined()) {
            ret = strings->undefined.data();
        } else if(isNull()) {
            ret = strings->null.data();
        } else if(isBoolean()) {
            if(asBoolean())
                ret = strings->stringTrue.data();
            else
                ret = strings->stringFalse.data();
        } else {
            ESPointer* o = asESPointer();
            if(o->isESString()) {
                return o->asESString();
            } else if(o->isESFunctionObject()) {
                //ret = L"[Function function]";
                ret = L"function ";
                ESFunctionObject* fn = o->asESFunctionObject();
                ret.append(fn->functionAST()->id().data());
                ret.append(L"() {}");
            } else if(o->isESArrayObject()) {
                bool isFirst = true;
                for (int i = 0 ; i < o->asESArrayObject()->length().asInt32() ; i++) {
                    if(!isFirst)
                        ret.append(L",");
                    ESValue slot = o->asESArrayObject()->get(i);
                    ret.append(slot.toInternalString().data());
                    isFirst = false;
                  }
            } else if(o->isESErrorObject()) {
                ret.append(o->asESObject()->get(L"name", true).toInternalString().data());
                ret.append(L": ");
                ret.append(o->asESObject()->get(L"message").toInternalString().data());
            } else if(o->isESObject()) {
                ret.append(L"[Object object]");
            } else {
                RELEASE_ASSERT_NOT_REACHED();
            }
        }

        return ESString::create(std::move(ret));
    } else {
        ASSERT(asESPointer()->isESObject());
        ESObject* obj = asESPointer()->asESObject();
        ESValue ret = ESFunctionObject::call(obj->get(strings->toString, true), obj, NULL, 0, ESVMInstance::currentInstance(), false);
        ASSERT(ret.isESString());
        return ret.asESString();
    }
}


ESObject* ESValue::toObject() const
{
    ESFunctionObject* function;
    ESObject* receiver;
    if (isNumber()) {
        function = ESVMInstance::currentInstance()->globalObject()->number();
        receiver = ESNumberObject::create(toNumber());
    } else if (isBoolean()) {
        function = ESVMInstance::currentInstance()->globalObject()->boolean();
        ESValue ret;
        if (toBoolean())
            ret = ESValue(ESValue::ESTrueTag::ESTrue);
        else
            ret = ESValue(ESValue::ESFalseTag::ESFalse);
        receiver = ESBooleanObject::create(ret);
    } else if (isESString()) {
        function = ESVMInstance::currentInstance()->globalObject()->string();
        receiver = ESStringObject::create(asESPointer()->asESString()->string());
    } else if (isESPointer() && asESPointer()->isESObject()) {
        return asESPointer()->asESObject();
    } else {
        RELEASE_ASSERT_NOT_REACHED();
    }
    receiver->setConstructor(function);
    receiver->set__proto__(function->protoType());
    /*
    std::vector<ESValue, gc_allocator<ESValue>> arguments;
    arguments.push_back(this);
    ESFunctionObject::call((ESValue) function, receiver, &arguments[0], arguments.size(), this);
    */
    return receiver;
}


ESValue ESObject::valueOf()
{
    if(isESDateObject())
        return asESDateObject()->valueOf();
    else if (isESStringObject())
        return asESStringObject()->valueOf();
    else if(isESArrayObject())
        // Array.prototype do not have valueOf() function
        RELEASE_ASSERT_NOT_REACHED();
    else
        RELEASE_ASSERT_NOT_REACHED();

    /*
    if (hint == ESValue::PreferString) {
        ESValue underScoreProto = get(strings->__proto__);
        ESValue toStringMethod = underScoreProto.asESPointer()->asESObject()->get(L"toString");
        std::vector<ESValue> arguments;
        return ESFunctionObject::call(toStringMethod, this, &arguments[0], arguments.size(), ESVMInstance::currentInstance());
    } else {
        //TODO
        RELEASE_ASSERT_NOT_REACHED();
    }
    */
}

InternalString ESValue::toInternalString() const
{
    InternalString ret;

    if(isInt32()) {
        int val = asInt32();
        if(val >= 0 && val < ESCARGOT_STRINGS_NUMBERS_MAX)
            ret = strings->nonAtomicNumbers[val];
        else ret = InternalString(val);
    } else if(isNumber()) {
        double d = asNumber();
        if (std::isnan(d)) ret = L"NaN";
        else if (d == std::numeric_limits<double>::infinity()) ret = L"Infinity";
        else if (d== -std::numeric_limits<double>::infinity()) ret = L"-Infinity";
        else ret = InternalString(d);
    } else if(isUndefined()) {
        ret = strings->undefined;
    } else if(isNull()) {
        ret = strings->null;
    } else if(isBoolean()) {
        if(asBoolean())
            ret = strings->stringTrue;
        else
            ret = strings->stringFalse;
    } else {
        ESPointer* o = asESPointer();
        if(o->isESString()) {
            ret = o->asESString()->string().data();
        } else if(o->isESFunctionObject()) {
            //ret = L"[Function function]";
            ret = L"function ";
            ESFunctionObject* fn = o->asESFunctionObject();
            ret.append(fn->functionAST()->id().data());
            ret.append(L"() {}");
        } else if(o->isESArrayObject()) {
            bool isFirst = true;
            for (int i = 0 ; i < o->asESArrayObject()->length().asInt32() ; i++) {
                if(!isFirst)
                    ret.append(L",");
                ESValue slot = o->asESArrayObject()->get(i);
                ret.append(slot.toInternalString());
                isFirst = false;
              }
        } else if(o->isESErrorObject()) {
            ret.append(o->asESObject()->get(L"name", true).toInternalString().data());
            ret.append(L": ");
            ret.append(o->asESObject()->get(L"message").toInternalString().data());
        } else if(o->isESObject()) {
            ret.append("[Object object]");
        } else {
            RELEASE_ASSERT_NOT_REACHED();
        }
    }

    return ret;
}

ESArrayObject* ESString::match(ESPointer* esptr, std::vector<int>* offsets) const
{
    escargot::ESArrayObject* ret = ESArrayObject::create(0, ESVMInstance::currentInstance()->globalObject()->arrayPrototype());

    const char* source;
    ESRegExpObject::Option option = ESRegExpObject::Option::None;
    if (esptr->isESRegExpObject()) {
        source = esptr->asESRegExpObject()->utf8Source();
        option = esptr->asESRegExpObject()->option();
    } else if (esptr->isESString()) {
        source = utf16ToUtf8(esptr->asESString()->string().data());
    } else {
        //TODO
        RELEASE_ASSERT_NOT_REACHED();
    }

    const char* targetString = utf16ToUtf8(m_string->data());
    int index = 0;

    ESRegExpObject::prepareForRE2(source, option, [&](const char* RE2Source, const re2::RE2::Options& ops, const bool& isGlobal){
        re2::RE2 re(RE2Source, ops);
        re2::StringPiece input(targetString);
        re2::StringPiece matched;
        const char* inputDataStart = input.data();
        while (re2::RE2::FindAndConsume(&input, re, &matched)) {
            if (offsets)
                offsets->push_back(matched.data() - inputDataStart);
            std::string std_matched(matched.data(), 0, matched.length());
            InternalString int_str(std_matched.data());
            ret->set(index, ESString::create(int_str));
            index++;
            if (!isGlobal)
                break;
        }
    });

    GC_free((char *)targetString);

    return ret;
}

ESObject::ESObject(ESPointer::Type type)
    : ESPointer(type)
    , m_map(16)
{
    //FIXME set proper flags(is...)
    definePropertyOrThrow(strings->constructor, true, false, false);
    defineAccessorProperty(strings->__proto__, ESVMInstance::currentInstance()->object__proto__AccessorData(), true, false, false);
}

ESArrayObject::ESArrayObject()
    : ESObject((Type)(Type::ESObject | Type::ESArrayObject))
    , m_vector(16)
    , m_fastmode(true)
{
    defineAccessorProperty(strings->length, ESVMInstance::currentInstance()->arrayLengthAccessorData(), true, false, false);
    m_length = 0;
}

ESRegExpObject::ESRegExpObject(const InternalString& source, const Option& option)
    : ESObject((Type)(Type::ESObject | Type::ESRegExpObject))
{
    m_source = source;
    m_option = option;
    m_sourceStringAsUtf8 = source.utf8Data();
}

ESFunctionObject::ESFunctionObject(LexicalEnvironment* outerEnvironment, FunctionNode* functionAST, ESObject* proto)
    : ESObject((Type)(Type::ESObject | Type::ESFunctionObject))
{
    m_outerEnvironment = outerEnvironment;
    m_functionAST = functionAST;
    defineAccessorProperty(strings->prototype, ESVMInstance::currentInstance()->functionPrototypeAccessorData(), true, false, false);

    if (proto != NULL)
        set__proto__(proto);
    else
        set__proto__(ESVMInstance::currentInstance()->globalFunctionPrototype());
}

ALWAYS_INLINE void functionCallerInnerProcess(ESFunctionObject* fn, ESValue receiver, ESValue arguments[], size_t argumentCount, bool needsArgumentsObject, ESVMInstance* ESVMInstance)
{
    //ESVMInstance->invalidateIdentifierCacheCheckCount();

    ((FunctionEnvironmentRecord *)ESVMInstance->currentExecutionContext()->environment()->record())->bindThisValue(receiver.asESPointer()->asESObject());
    DeclarativeEnvironmentRecord* functionRecord = ESVMInstance->currentExecutionContext()->environment()->record()->toDeclarativeEnvironmentRecord();

    if(UNLIKELY(fn->functionAST()->needsActivation())) {
        const InternalAtomicStringVector& params = fn->functionAST()->params();
        const InternalStringVector& nonAtomicParams = fn->functionAST()->nonAtomicParams();
        for(unsigned i = 0; i < params.size() ; i ++) {
            functionRecord->createMutableBinding(params[i], nonAtomicParams[i], false);
            if(i < argumentCount) {
                functionRecord->setMutableBinding(params[i], nonAtomicParams[i], arguments[i], true);
            }
        }
    } else {
        const InternalAtomicStringVector& params = fn->functionAST()->params();
        const InternalStringVector& nonAtomicParams = fn->functionAST()->nonAtomicParams();
        for(unsigned i = 0; i < params.size() ; i ++) {
            if(i < argumentCount) {
                functionRecord->setMutableBinding(params[i], nonAtomicParams[i], arguments[i], true);
            }
        }
    }

    if(UNLIKELY(needsArgumentsObject)) {
        ESObject* argumentsObject = ESObject::create();
        unsigned i = 0;
        argumentsObject->set(strings->length, ESValue(argumentCount));
        for(; i < argumentCount && i < ESCARGOT_STRINGS_NUMBERS_MAX ; i ++) {
            argumentsObject->set(strings->nonAtomicNumbers[i], arguments[i]);
        }
        for( ; i < argumentCount ; i ++) {
            argumentsObject->set(InternalString((int)i).data(), arguments[i]);
        }

        if(!fn->functionAST()->needsActivation()) {
            for(size_t i = 0 ; i < fn->functionAST()->innerIdentifiers().size() ; i++ ) {
                if(fn->functionAST()->innerIdentifiers()[i] == strings->atomicArguments) {
                    functionRecord->getBindingValueForNonActivationMode(i)->setDataProperty(argumentsObject);
                    break;
                }
            }
            ASSERT(functionRecord->hasBinding(strings->atomicArguments, L"arguments"));
        } else {
            functionRecord->createMutableBinding(strings->atomicArguments, strings->arguments, false);
            functionRecord->setMutableBinding(strings->atomicArguments, strings->arguments, argumentsObject, true);
        }
    }

}


ESValue ESFunctionObject::call(ESValue callee, ESValue receiver, ESValue arguments[], size_t argumentCount, ESVMInstance* ESVMInstance, bool isNewExpression)
{
    ESValue result;
    if(callee.isESPointer() && callee.asESPointer()->isESFunctionObject()) {
        ExecutionContext* currentContext = ESVMInstance->currentExecutionContext();
        ESFunctionObject* fn = callee.asESPointer()->asESFunctionObject();
        if(fn->functionAST()->needsActivation()) {
            ESVMInstance->m_currentExecutionContext = new ExecutionContext(LexicalEnvironment::newFunctionEnvironment(fn, receiver), true, isNewExpression, currentContext, arguments, argumentCount);
            functionCallerInnerProcess(fn, receiver, arguments, argumentCount, true, ESVMInstance);
            int r = setjmp(ESVMInstance->currentExecutionContext()->returnPosition());
            if(r != 1) {
                fn->functionAST()->body()->execute(ESVMInstance);
            }
            result = ESVMInstance->currentExecutionContext()->returnValue();
            ESVMInstance->m_currentExecutionContext = currentContext;
        } else {
            FunctionEnvironmentRecord envRec(true,
                    (::escargot::ESSlot *)alloca(sizeof(::escargot::ESSlot) * fn->functionAST()->innerIdentifiers().size()),
                    fn->functionAST(),
                    &fn->functionAST()->innerIdentifiers());

            envRec.m_functionObject = fn;
            envRec.m_newTarget = receiver;

            LexicalEnvironment env(&envRec, fn->outerEnvironment());
            ExecutionContext ec(&env, false, isNewExpression, currentContext, arguments, argumentCount);
            ESVMInstance->m_currentExecutionContext = &ec;
            functionCallerInnerProcess(fn, receiver, arguments, argumentCount, fn->functionAST()->needsArgumentsObject(), ESVMInstance);
            int r = setjmp(ESVMInstance->currentExecutionContext()->returnPosition());
            if(r != 1) {
                fn->functionAST()->body()->execute(ESVMInstance);
            }
            result = ESVMInstance->currentExecutionContext()->returnValue();
            ESVMInstance->m_currentExecutionContext = currentContext;
        }
    } else {
        throw TypeError(L"Callee is not a function object");
    }

    return result;
}

void ESDateObject::parseStringToDate(struct tm* timeinfo, const InternalString istr) {
    int len = istr.length();
    const wchar_t* wc = istr.data();
    char buffer[len];

    wcstombs(buffer, wc, sizeof(buffer));
    if (isalpha(wc[0])) {
        strptime(buffer, "%B %d %Y %H:%M:%S %z", timeinfo);
    } else if (isdigit(wc[0])) {
        strptime(buffer, "%m/%d/%Y %H:%M:%S", timeinfo);
    }
}

void ESDateObject::setTimeValue(ESValue str) {
    if (str.isUndefined()) {
        gettimeofday(&m_tv, NULL);
    } else {
        time_t rawtime;
        struct tm* timeinfo = localtime(&rawtime);;
        InternalString istr = str.asESString()->string().data();
        parseStringToDate(timeinfo, istr);

        m_tv.tv_sec = mktime(timeinfo);
    }
}

int ESDateObject::getDate() {
    return localtime(&(m_tv.tv_sec))->tm_mday;
}

int ESDateObject::getDay() {
    return localtime(&(m_tv.tv_sec))->tm_wday;
}

int ESDateObject::getFullYear() {
    return localtime(&(m_tv.tv_sec))->tm_year + 1900;
}

int ESDateObject::getHours() {
    return localtime(&(m_tv.tv_sec))->tm_hour;
}

int ESDateObject::getMinutes() {
    return localtime(&(m_tv.tv_sec))->tm_min;
}

int ESDateObject::getMonth() {
    return localtime(&(m_tv.tv_sec))->tm_mon;
}

int ESDateObject::getSeconds() {
    return localtime(&(m_tv.tv_sec))->tm_sec;
}

int ESDateObject::getTimezoneOffset() {
//    return localtime(&(m_tv.tv_sec))->tm_gmtoff;
//    const time_t a = m_tv.tv_sec;
//    struct tm* b = gmtime(&a);
//    time_t c = mktime(b);
//    int d = c - a;
    return -540;
}

void ESDateObject::setTime(double t) {
    time_t raw_t = (time_t) floor(t);
    m_tv.tv_sec = raw_t/1000;
    m_tv.tv_usec =  (raw_t % 10000) * 1000;
}

ESStringObject::ESStringObject(const InternalStringStd& str)
    : ESObject((Type)(Type::ESObject | Type::ESStringObject))
{
    m_stringData = ESString::create(str);

    //$21.1.4.1 String.length
    defineAccessorProperty(strings->length, ESVMInstance::currentInstance()->stringObjectLengthAccessorData(), false, true, false);
}

}
