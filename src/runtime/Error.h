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

#ifndef Error_h
#define Error_h

namespace escargot {

extern const char* errorMessage_DefineProperty_Default;
extern const char* errorMessage_DefineProperty_LengthNotWritable;
extern const char* errorMessage_DefineProperty_NotWritable;
extern const char* errorMessage_DefineProperty_RedefineNotConfigurable;
extern const char* errorMessage_DefineProperty_NotExtensible;
extern const char* errorMessage_GlobalObject_ThisUndefinedOrNull;
extern const char* errorMessage_GlobalObject_ThisNotObject;
extern const char* errorMessage_GlobalObject_ThisNotRegExpObject;
extern const char* errorMessage_GlobalObject_ThisNotDateObject;
extern const char* errorMessage_GlobalObject_ThisNotFunctionObject;
extern const char* errorMessage_GlobalObject_ThisNotBoolean;
extern const char* errorMessage_GlobalObject_ThisNotNumber;
extern const char* errorMessage_GlobalObject_ThisNotString;
extern const char* errorMessage_GlobalObject_ThisNotTypedArrayObject;
extern const char* errorMessage_GlobalObject_MalformedURI;
extern const char* errorMessage_GlobalObject_FileNotExist;
extern const char* errorMessage_GlobalObject_NotExecutable;
extern const char* errorMessage_GlobalObject_FirstArgumentNotObject;
extern const char* errorMessage_GlobalObject_SecondArgumentNotObject;
extern const char* errorMessage_GlobalObject_DescriptorNotObject;
extern const char* errorMessage_GlobalObject_ToLoacleStringNotCallable;
extern const char* errorMessage_GlobalObject_ToISOStringNotCallable;
extern const char* errorMessage_GlobalObject_CallbackNotCallable;
extern const char* errorMessage_GlobalObject_InvalidDate;
extern const char* errorMessage_GlobalObject_JAError;
extern const char* errorMessage_GlobalObject_JOError;
extern const char* errorMessage_GlobalObject_RadixInvalidRange;
extern const char* errorMessage_GlobalObject_NotDefineable;
extern const char* errorMessage_GlobalObject_FirstArgumentNotObjectAndNotNull;
extern const char* errorMessage_GlobalObject_FirstArgumentNotCallable;
extern const char* errorMessage_GlobalObject_FirstArgumentNotString;
extern const char* errorMessage_GlobalObject_FirstArgumentInvalidLength;
extern const char* errorMessage_GlobalObject_InvalidArrayBufferOffset;
extern const char* errorMessage_GlobalObject_ReduceError;
extern const char* errorMessage_GlobalObject_NotExistNewInArrayBufferConstructor;
extern const char* errorMessage_GlobalObject_NotExistNewInTypedArrayConstructor;
extern const char* errorMessage_GlobalObject_RangeError;
extern const char* errorMessage_GlobalObject_InvalidArrayLength;
extern const char* errorMessage_CallStackExceeded;
extern const char* errorMessage_ObjectToPrimitiveValue;
extern const char* errorMessage_UndefinedToObject;
extern const char* errorMessage_NullToObject;
extern const char* errorMessage_New_NotConstructor;
extern const char* errorMessage_New_NotFunction;
extern const char* errorMessage_InstanceOf_NotFunction;
extern const char* errorMessage_InstanceOf_InvalidPrototypeProperty;
extern const char* errorMessage_Call_NotFunction;
extern const char* errorMessage_Get_FromUndefined;
extern const char* errorMessage_Get_FromNull;
extern const char* errorMessage_Set_ToUndefined;
extern const char* errorMessage_Set_ToNull;
extern const char* errorMessage_ArgumentsOrCaller_InStrictMode;
}

#endif
