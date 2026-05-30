#include "ast.hpp"
#include "globals.hpp"
#include <llvm/IR/Verifier.h>
#include <map>
#include <stdexcept>

namespace {
std::vector<std::map<std::string, llvm::Value*>> symbolScopes;

std::map<std::string, llvm::Value*>& currentScope() {
    if (symbolScopes.empty()) {
        symbolScopes.emplace_back();
    }
    return symbolScopes.back();
}

llvm::Value* lookupSymbol(const std::string& name) {
    for (auto scope = symbolScopes.rbegin(); scope != symbolScopes.rend(); ++scope) {
        auto found = scope->find(name);
        if (found != scope->end()) {
            return found->second;
        }
    }
    return nullptr;
}

llvm::Value* coerceToInt32(llvm::IRBuilder<>& builder, llvm::Value* value) {
    if (value->getType()->isIntegerTy(32)) {
        return value;
    }
    if (value->getType()->isIntegerTy(1)) {
        return builder.CreateZExt(value, builder.getInt32Ty());
    }
    throw std::runtime_error("Expected integer expression");
}
}

llvm::Value* FuncDeclNode::codegen(llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::vector<llvm::Type*> paramTypes(params.size(), builder.getInt32Ty());
    auto* funcType = llvm::FunctionType::get(builder.getInt32Ty(), paramTypes, false);
    auto* func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, module);
    auto* entry = llvm::BasicBlock::Create(module.getContext(), "entry", func);
    builder.SetInsertPoint(entry);
    symbolScopes.emplace_back();
    auto argIt = func->arg_begin();
    for (const auto& param : params) {
        auto* alloc = builder.CreateAlloca(builder.getInt32Ty(), nullptr, param);
        builder.CreateStore(&*argIt, alloc);
        currentScope()[param] = alloc;
        ++argIt;
    }
    for (auto* stmt : body) {
        stmt->codegen(builder, module);
    }
    if (!builder.GetInsertBlock()->getTerminator()) {
        builder.CreateRet(llvm::ConstantInt::get(builder.getInt32Ty(), 0));
    }
    llvm::verifyFunction(*func);
    symbolScopes.pop_back();
    return func;
}

llvm::Value* VarDeclNode::codegen(llvm::IRBuilder<>& builder, llvm::Module& module) {
    auto* alloc = builder.CreateAlloca(builder.getInt32Ty(), nullptr, name);
    currentScope()[name] = alloc;
    if (init) {
        auto* value = coerceToInt32(builder, init->codegen(builder, module));
        builder.CreateStore(value, alloc);
    }
    return alloc;
}

llvm::Value* AssignmentNode::codegen(llvm::IRBuilder<>& builder, llvm::Module& module) {
    auto* alloc = lookupSymbol(name);
    if (!alloc) throw std::runtime_error("Undeclared variable: " + name);
    auto* value = coerceToInt32(builder, expr->codegen(builder, module));
    return builder.CreateStore(value, alloc);
}

llvm::Value* RaiseNode::codegen(llvm::IRBuilder<>& builder, llvm::Module& module) {
    auto& context = module.getContext();
    auto* int8PtrType = llvm::PointerType::get(llvm::Type::getInt8Ty(context), 0);
    auto throwFuncCallee = module.getOrInsertFunction(
        "bateman_throw",
        llvm::FunctionType::get(builder.getVoidTy(), llvm::ArrayRef<llvm::Type*>{int8PtrType}, false)
    );
    auto* throwFunc = llvm::cast<llvm::Function>(throwFuncCallee.getCallee());
    auto* str = builder.CreateGlobalStringPtr(message);
    builder.CreateCall(throwFunc, {str});
    builder.CreateUnreachable();
    return nullptr;
}

llvm::Value* IfNode::codegen(llvm::IRBuilder<>& builder, llvm::Module& module) {
    auto* conditionValue = condition->codegen(builder, module);
    if (conditionValue->getType()->isIntegerTy(32)) {
        conditionValue = builder.CreateICmpNE(conditionValue, llvm::ConstantInt::get(builder.getInt32Ty(), 0));
    } else if (!conditionValue->getType()->isIntegerTy(1)) {
        throw std::runtime_error("If condition must be an integer expression");
    }

    auto* function = builder.GetInsertBlock()->getParent();
    auto* thenBlock = llvm::BasicBlock::Create(module.getContext(), "if.then", function);
    auto* continuationBlock = llvm::BasicBlock::Create(module.getContext(), "if.end");

    builder.CreateCondBr(conditionValue, thenBlock, continuationBlock);
    builder.SetInsertPoint(thenBlock);
    for (auto* stmt : body) {
        stmt->codegen(builder, module);
    }
    if (!builder.GetInsertBlock()->getTerminator()) {
        builder.CreateBr(continuationBlock);
    }

    function->insert(function->end(), continuationBlock);
    builder.SetInsertPoint(continuationBlock);
    return continuationBlock;
}

llvm::Value* PrintNode::codegen(llvm::IRBuilder<>& builder, llvm::Module& module) {
    auto printfCallee = module.getOrInsertFunction("printf",
        llvm::FunctionType::get(builder.getInt32Ty(),
                               {llvm::PointerType::get(builder.getInt8Ty(), 0)},
                               true));
    auto* printfFunc = llvm::cast<llvm::Function>(printfCallee.getCallee());

    auto* value = expr->codegen(builder, module);
    if (value->getType()->isPointerTy()) {
        auto* strPtr = builder.CreateBitCast(value, llvm::PointerType::get(builder.getInt8Ty(), 0));
        builder.CreateCall(printfFunc, {strPtr});
        return nullptr;
    }

    value = coerceToInt32(builder, value);
    auto* format = builder.CreateGlobalStringPtr("%d\n");
    builder.CreateCall(printfFunc, {format, value});
    return nullptr;
}

llvm::Value* CallNode::codegen(llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::vector<llvm::Value*> args;
    for (auto* arg : this->args) {
        auto* value = coerceToInt32(builder, arg->codegen(builder, module));
        args.push_back(value);
    }
    auto* func = module.getFunction(name);
    if (!func) {
        auto funcCallee = module.getOrInsertFunction(name,
            llvm::FunctionType::get(builder.getInt32Ty(), std::vector<llvm::Type*>(args.size(), builder.getInt32Ty()), false));
        func = llvm::cast<llvm::Function>(funcCallee.getCallee());
    }
    return builder.CreateCall(func, args);
}

llvm::Value* ReturnNode::codegen(llvm::IRBuilder<>& builder, llvm::Module& module) {
    auto* value = coerceToInt32(builder, expr->codegen(builder, module));
    return builder.CreateRet(value);
}

llvm::Value* InputNode::codegen(llvm::IRBuilder<>& builder, llvm::Module& module) {
    auto* alloc = lookupSymbol(name);
    if (!alloc) {
        alloc = builder.CreateAlloca(builder.getInt32Ty(), nullptr, name);
        currentScope()[name] = alloc;
    }
    auto readFuncCallee = module.getOrInsertFunction("bateman_read",
        llvm::FunctionType::get(builder.getInt32Ty(), false));
    auto* readFunc = llvm::cast<llvm::Function>(readFuncCallee.getCallee());
    auto* value = builder.CreateCall(readFunc);
    return builder.CreateStore(value, alloc);
}

llvm::Value* NumberNode::codegen(llvm::IRBuilder<>& builder, llvm::Module& module) {
    return llvm::ConstantInt::get(builder.getInt32Ty(), value);
}

llvm::Value* StringNode::codegen(llvm::IRBuilder<>& builder, llvm::Module& module) {
    return builder.CreateGlobalStringPtr(value, "str");
}

llvm::Value* IdentNode::codegen(llvm::IRBuilder<>& builder, llvm::Module& module) {
    auto* alloc = lookupSymbol(name);
    if (!alloc) throw std::runtime_error("Undeclared variable: " + name);
    return builder.CreateLoad(builder.getInt32Ty(), alloc, name);
}

llvm::Value* BinOpNode::codegen(llvm::IRBuilder<>& builder, llvm::Module& module) {
    auto* leftValue = coerceToInt32(builder, left->codegen(builder, module));
    auto* rightValue = coerceToInt32(builder, right->codegen(builder, module));
    if (op == "+") return builder.CreateAdd(leftValue, rightValue);
    if (op == "-") return builder.CreateSub(leftValue, rightValue);
    if (op == "*") return builder.CreateMul(leftValue, rightValue);
    if (op == "/") return builder.CreateSDiv(leftValue, rightValue);
    if (op == "==") return builder.CreateICmpEQ(leftValue, rightValue);
    throw std::runtime_error("Unsupported operator: " + op);
}