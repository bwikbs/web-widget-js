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

#ifdef ENABLE_ESJIT

#include "Escargot.h"
#include "ESJITMiddleend.h"

#include "ESGraph.h"
#include "ESIR.h"

namespace escargot {
namespace ESJIT {

// In interpreter, we only profile the type of 1. the variable loaded from the
// heap, 2. function argument, and 3. return value of function call.
// For other IRs, We should run type inference phase to examine the types.

bool ESGraphTypeInference::run(ESGraph* graph)
{
    for (size_t i = 0; i < graph->basicBlockSize(); i++) {
        ESBasicBlock* block = graph->basicBlock(i);
        for (size_t j = 0; j < block->instructionSize(); j++) {
            ESIR* ir = block->instruction(j);
            switch (ir->opcode()) {
#define INIT_ESIR(opcode) \
                opcode##IR* ir##opcode = static_cast<opcode##IR*>(ir); \
                (void)ir##opcode;
            case ESIR::Opcode::ConstantESValue:
                {
                    INIT_ESIR(ConstantESValue);
                    break;
                }
            case ESIR::Opcode::ConstantInt:
                {
                    INIT_ESIR(ConstantInt);
                    if (graph->getOperandType(irConstantInt->targetIndex()).isDoubleType())
                        block->replace(j, ConstantDoubleIR::create(irConstantInt->targetIndex(), (double) irConstantInt->value()));
                    break;
                }
            case ESIR::Opcode::ConstantDouble:
                break;
            case ESIR::Opcode::ConstantPointer:
                break;
            case ESIR::Opcode::ConstantBoolean:
                break;
            case ESIR::Opcode::ConstantString:
                break;
            case ESIR::Opcode::ToNumber:
                {
                    INIT_ESIR(ToNumber);
                    Type srcType = graph->getOperandType(irToNumber->sourceIndex());
                    if (srcType.isInt32Type())
                        graph->setOperandType(ir->targetIndex(), TypeInt32);
                    else
                        graph->setOperandType(ir->targetIndex(), TypeDouble);
                    break;
                }
            case ESIR::Opcode::GenericPlus:
                {
                    INIT_ESIR(GenericPlus);
                    Type leftType = graph->getOperandType(irGenericPlus->leftIndex());
                    Type rightType = graph->getOperandType(irGenericPlus->rightIndex());
                    if (leftType.isInt32Type() && rightType.isInt32Type()) {
                        ESIR* int32PlusIR = Int32PlusIR::create(irGenericPlus->targetIndex(), irGenericPlus->leftIndex(), irGenericPlus->rightIndex());
                        block->replace(j, int32PlusIR);
                        graph->setOperandType(int32PlusIR->targetIndex(), TypeInt32);
                    } else if (leftType.isNumberType() && rightType.isNumberType()) {
                        ESIR* doublePlusIR = DoublePlusIR::create(irGenericPlus->targetIndex(), irGenericPlus->leftIndex(), irGenericPlus->rightIndex());
                        block->replace(j, doublePlusIR);
                        graph->setOperandType(doublePlusIR->targetIndex(), TypeDouble);
                    } else if (leftType.isStringType() || rightType.isStringType()) {
                        ESIR* stringPlusIR = StringPlusIR::create(irGenericPlus->targetIndex(), irGenericPlus->leftIndex(), irGenericPlus->rightIndex());
                        block->replace(j, stringPlusIR);
                        graph->setOperandType(stringPlusIR->targetIndex(), TypeString);
                    } else if (leftType.isUndefinedType() || rightType.isUndefinedType()) {
                        graph->setOperandType(ir->targetIndex(), TypeDouble);
                    } else {
                        printf("Unhandled GenericPlus case in ESGraphTypeInference (block %zu instruction %zu)\n", i, j);
                        RELEASE_ASSERT_NOT_REACHED();
                    }
                    break;
                }
            case ESIR::Opcode::Increment:
                {
                    INIT_ESIR(Increment);
                    Type srcType = graph->getOperandType(irIncrement->sourceIndex());
                    if (srcType.isInt32Type())
                        graph->setOperandType(ir->targetIndex(), TypeInt32);
                    else if (srcType.isDoubleType())
                        graph->setOperandType(ir->targetIndex(), TypeDouble);
                    else
                        RELEASE_ASSERT_NOT_REACHED();
                    break;
                }
            case ESIR::Opcode::Decrement:
                {
                    INIT_ESIR(Decrement);
                    Type srcType = graph->getOperandType(irDecrement->sourceIndex());
                    if (srcType.isInt32Type())
                        graph->setOperandType(ir->targetIndex(), TypeInt32);
                    else if (srcType.isDoubleType())
                        graph->setOperandType(ir->targetIndex(), TypeDouble);
                    else
                        RELEASE_ASSERT_NOT_REACHED();
                    break;
                }
            case ESIR::Opcode::Minus:
                {
                    INIT_ESIR(Minus);
                    Type leftType = graph->getOperandType(irMinus->leftIndex());
                    Type rightType = graph->getOperandType(irMinus->rightIndex());

                    // FIXME : Question> need to seperate IR? (e.g. DoubleMinus, Int32Minus)
                    if (leftType.isNumberType() && rightType.isNumberType()) {
                        if (leftType.isInt32Type() && rightType.isInt32Type())
                            graph->setOperandType(ir->targetIndex(), TypeInt32);
                        else
                            graph->setOperandType(ir->targetIndex(), TypeDouble);
                    } else
                        graph->setOperandType(ir->targetIndex(), TypeInt32); // FIXME : can be Double
                    break;
                }
            case ESIR::Opcode::GenericMultiply:
                {
                    INIT_ESIR(GenericMultiply);
                    Type leftType = graph->getOperandType(irGenericMultiply->leftIndex());
                    Type rightType = graph->getOperandType(irGenericMultiply->rightIndex());
                    if (leftType.isInt32Type() && rightType.isInt32Type()) {
                        // int32 * int32 = int32 (or Double -> OSR Exit)
                        ESIR* int32MultiplyIR = Int32MultiplyIR::create(irGenericMultiply->targetIndex(), irGenericMultiply->leftIndex(), irGenericMultiply->rightIndex());
                        block->replace(j, int32MultiplyIR);
                        graph->setOperandType(int32MultiplyIR->targetIndex(), TypeDouble);
                    } else if (leftType.isNumberType() && rightType.isNumberType()) {
                        // int32 * Double  = Double
                        // Double * int32  = Double
                        // Double * Double = Double
                        ESIR* doubleMultiplyIR = DoubleMultiplyIR::create(irGenericMultiply->targetIndex(), irGenericMultiply->leftIndex(), irGenericMultiply->rightIndex());
                        block->replace(j, doubleMultiplyIR);
                        graph->setOperandType(doubleMultiplyIR->targetIndex(), TypeDouble);
                    } else {
                        // FIXME
                        // Handle unusual case of multiply
                        graph->setOperandType(irGenericMultiply->targetIndex(), TypeDouble);
                    }
                    break;
                }
            case ESIR::Opcode::GenericDivision:
                {
                    INIT_ESIR(GenericDivision);
                    Type leftType = graph->getOperandType(irGenericDivision->leftIndex());
                    Type rightType = graph->getOperandType(irGenericDivision->rightIndex());
                    if (leftType.isNumberType() && rightType.isNumberType()) {
                        ESIR* doubleDivisionIR = DoubleDivisionIR::create(irGenericDivision->targetIndex(), irGenericDivision->leftIndex(), irGenericDivision->rightIndex());
                        block->replace(j, doubleDivisionIR);
                        graph->setOperandType(doubleDivisionIR->targetIndex(), TypeDouble);
                    } else {
                        // FIXME
                        // Handle unusual case of division
                        graph->setOperandType(irGenericDivision->targetIndex(), TypeDouble);
                    }
                    break;
                }
            case ESIR::Opcode::GenericMod:
                {
                    INIT_ESIR(GenericMod);
                    Type leftType = graph->getOperandType(irGenericMod->leftIndex());
                    Type rightType = graph->getOperandType(irGenericMod->rightIndex());
                    if (leftType.isInt32Type() && rightType.isInt32Type()) {
                        ESIR* int32ModIR = Int32ModIR::create(irGenericMod->targetIndex(), irGenericMod->leftIndex(), irGenericMod->rightIndex());
                        block->replace(j, int32ModIR);
                        graph->setOperandType(int32ModIR->targetIndex(), TypeInt32);
                    } else if (leftType.isNumberType() && rightType.isNumberType()) {
                        ESIR* doubleModIR = DoubleModIR::create(irGenericMod->targetIndex(), irGenericMod->leftIndex(), irGenericMod->rightIndex());
                        block->replace(j, doubleModIR);
                        graph->setOperandType(doubleModIR->targetIndex(), TypeDouble);
                    } else {
                        // FIXME
                        // Handle unusual case of division
                        graph->setOperandType(irGenericMod->targetIndex(), TypeDouble);
                    }
                    break;
                }
            case ESIR::Opcode::BitwiseAnd:
            case ESIR::Opcode::BitwiseOr:
            case ESIR::Opcode::BitwiseXor:
            case ESIR::Opcode::BitwiseNot:
                graph->setOperandType(ir->targetIndex(), TypeInt32);
                break;
            case ESIR::Opcode::LogicalNot:
            case ESIR::Opcode::Equal:
            case ESIR::Opcode::NotEqual:
            case ESIR::Opcode::StrictEqual:
            case ESIR::Opcode::NotStrictEqual:
            case ESIR::Opcode::GreaterThan:
            case ESIR::Opcode::GreaterThanOrEqual:
            case ESIR::Opcode::LessThan:
            case ESIR::Opcode::LessThanOrEqual:
                graph->setOperandType(ir->targetIndex(), TypeBoolean);
                break;
            case ESIR::Opcode::LeftShift:
            case ESIR::Opcode::SignedRightShift:
            case ESIR::Opcode::UnsignedRightShift:
                graph->setOperandType(ir->targetIndex(), TypeInt32);
                break;
            case ESIR::Opcode::UnaryMinus:
                {
                    INIT_ESIR(UnaryMinus);
                    Type srcType = graph->getOperandType(irUnaryMinus->sourceIndex());
                    if (srcType.isInt32Type())
                        graph->setOperandType(ir->targetIndex(), TypeInt32);
                    else if (srcType.isDoubleType())
                        graph->setOperandType(ir->targetIndex(), TypeDouble);
                    else
                        RELEASE_ASSERT_NOT_REACHED();
                    break;
                }
            case ESIR::Opcode::TypeOf:
                {
                    INIT_ESIR(TypeOf);
                    graph->setOperandType(irTypeOf->targetIndex(), TypeSimpleString);
                    break;
                }
            case ESIR::Opcode::CreateFunction:
                {
                    INIT_ESIR(CreateFunction);
                    if (irCreateFunction->targetIndex() >= 0)
                        graph->setOperandType(irCreateFunction->targetIndex(), TypeFunctionObject);
                    break;
                }
            case ESIR::Opcode::Jump:
            case ESIR::Opcode::Branch:
            case ESIR::Opcode::CallJS:
            case ESIR::Opcode::CallNewJS:
            case ESIR::Opcode::CallEval:
            case ESIR::Opcode::Return:
            case ESIR::Opcode::ReturnWithValue:
                break;
            case ESIR::Opcode::Move:
                {
                    INIT_ESIR(Move);
                    Type srcType = graph->getOperandType(irMove->sourceIndex());
                    graph->setOperandType(ir->targetIndex(), srcType);
                    break;
                }
            case ESIR::Opcode::GetEnumerablObjectData:
                {
                    graph->setOperandType(ir->targetIndex(), TypeObject);
                    break;
                }
            case ESIR::Opcode::GetEnumerateKey:
                break;
            case ESIR::Opcode::CheckIfKeyIsLast:
                {
                    graph->setOperandType(ir->targetIndex(), TypeBoolean);
                    break;
                }
            case ESIR::Opcode::GetThis:
            case ESIR::Opcode::GetArgument:
            case ESIR::Opcode::GetVar:
            case ESIR::Opcode::GetVarGeneric:
            case ESIR::Opcode::GetVarGenericWithoutException:
            case ESIR::Opcode::GetGlobalVarGeneric:
                break;
            case ESIR::Opcode::GetObject:
                {
                    INIT_ESIR(GetObject);
                    Type objectType = graph->getOperandType(irGetObject->objectIndex());
                    Type propertyType = graph->getOperandType(irGetObject->propertyIndex());
                    if (objectType.isArrayObjectType()) {
                        if (propertyType.isInt32Type() || propertyType.isDoubleType()) {
                            GetArrayObjectIR* getArrayObjectIR = GetArrayObjectIR::create(irGetObject->targetIndex(), irGetObject->objectIndex(), irGetObject->propertyIndex());
                            block->replace(j, getArrayObjectIR);
                        }
                    } else if (objectType.isStringType()) {
                        if (propertyType.isInt32Type()) {
                            GetStringByIndexIR* newIR = GetStringByIndexIR::create(irGetObject->targetIndex(), irGetObject->objectIndex(), irGetObject->propertyIndex());
                            block->replace(j, newIR);
                        }
                    } else {
                    }
                    break;
                }
            case ESIR::Opcode::SetObject:
                {
                    INIT_ESIR(SetObject);
                    Type srcType = graph->getOperandType(irSetObject->sourceIndex());
                    graph->setOperandType(ir->targetIndex(), srcType);
                    Type objectType = graph->getOperandType(irSetObject->objectIndex());
                    Type propertyType = graph->getOperandType(irSetObject->propertyIndex());
                    if (objectType.isArrayObjectType()) {
                        if (propertyType.isInt32Type() || propertyType.isDoubleType()) {
                            SetArrayObjectIR* setArrayObjectIR = SetArrayObjectIR::create(irSetObject->targetIndex(), irSetObject->objectIndex(), irSetObject->propertyIndex(), irSetObject->sourceIndex());
                            block->replace(j, setArrayObjectIR);
                        }
                    } else {
                    }
                    break;
                }
            case ESIR::Opcode::SetObjectPreComputed:
                {
                    INIT_ESIR(SetObjectPreComputed);
                    Type srcType = graph->getOperandType(irSetObjectPreComputed->sourceIndex());
                    graph->setOperandType(ir->targetIndex(), srcType);
                    Type objectType = graph->getOperandType(irSetObjectPreComputed->objectIndex());
                    break;
                }
            case ESIR::Opcode::GetArrayObject:
                break;
            case ESIR::Opcode::GetObjectPreComputed:
                break;
            case ESIR::Opcode::SetVar:
                {
                    SetVarIR* irSetVar = static_cast<SetVarIR*>(ir);
                    Type setType = graph->getOperandType(irSetVar->sourceIndex());
                    break;
                }
            case ESIR::Opcode::SetVarGeneric:
                {
                    INIT_ESIR(SetVarGeneric);
                    Type setType = graph->getOperandType(irSetVarGeneric->sourceIndex());
                    break;
                }
            case ESIR::Opcode::SetGlobalVarGeneric:
                {
                    INIT_ESIR(SetGlobalVarGeneric);
                    Type setType = graph->getOperandType(irSetGlobalVarGeneric->sourceIndex());
                    break;
                }
            case ESIR::Opcode::CreateObject:
                {
                    graph->setOperandType(ir->targetIndex(), Type(TypeObject));
                    break;
                }
            case ESIR::Opcode::CreateArray:
                {
                    graph->setOperandType(ir->targetIndex(), Type(TypeArrayObject));
                    break;
                }
            case ESIR::Opcode::InitObject:
            case ESIR::Opcode::InitArrayObject:
                break;
            case ESIR::Opcode::AllocPhi:
                break;
            case ESIR::Opcode::StorePhi:
                {
                    INIT_ESIR(StorePhi);
                    Type srcType = graph->getOperandType(irStorePhi->sourceIndex());
                    graph->setOperandType(ir->targetIndex(), srcType);
                    break;
                }
            case ESIR::Opcode::LoadPhi:
                {
                    INIT_ESIR(LoadPhi);
                    Type src0Type = graph->getOperandType(irLoadPhi->srcIndex0());
                    Type src1Type = graph->getOperandType(irLoadPhi->srcIndex1());
                    if (src0Type != src1Type) {
                        // TODO
                        goto unsupported;
                    }
                    graph->setOperandType(ir->targetIndex(), src0Type);
                    break;
                }
            case ESIR::Opcode::Throw:
                break;
            default:
                {
                    printf("ERROR %s not handled in ESGraphTypeInference.\n", ir->getOpcodeName());
                    RELEASE_ASSERT_NOT_REACHED();
                    // for complie
                    goto unsupported;
                }
            }
            continue;
            unsupported:
                LOG_VJ("Unsupported case in TypeInference (IR %s)\n", ir->getOpcodeName());
                return false;
        }
    }

#ifndef NDEBUG
    if (ESVMInstance::currentInstance()->m_verboseJIT)
        graph->dump(std::cout, "After running Type Inference");
#endif

    return true;
}

}}
#endif
