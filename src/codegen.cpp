#include "ast.hpp"
#include "globals.hpp"
#include <llvm/IR/Verifier.h>
#include <map>

std::map<std::string, llvm::Value*> symbolTable;

llvm::Value* FuncDeclNode::codegen(llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::vector<llvm::Type*> paramTypes(params.size(), builder.getInt32Ty());
    auto* funcType = llvm::FunctionType::get(builder.getInt32Ty(), paramTypes, false);
    auto* func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, module);
    auto* entry = llvm::BasicBlock::Create(module.getContext(), "entry", func);
    builder.SetInsertPoint(entry);
    auto argIt = func->arg_begin();
    for (const auto& param : params) {
        auto* alloc = builder.CreateAlloca(builder.getInt32Ty(), nullptr, param);
        builder.CreateStore(&*argIt, alloc);
        symbolTable[param] = alloc;
        ++argIt;
    }
    for (auto* stmt : body) {
        stmt->codegen(builder, module);
    }
    if (!builder.GetInsertBlock()->getTerminator()) {
        builder.CreateRet(llvm::ConstantInt::get(builder.getInt32Ty(), 0));
    }
    llvm::verifyFunction(*func);
    return func;
}

llvm::Value* VarDeclNode::codegen(llvm::IRBuilder<>& builder, llvm::Module& module) {
    auto* alloc = builder.CreateAlloca(builder.getInt32Ty(), nullptr, name);
    symbolTable[name] = alloc;
    if (init) {
        auto* value = init->codegen(builder, module);
        builder.CreateStore(value, alloc);
    }
    return alloc;
}

llvm::Value* RaiseNode::codegen(llvm::IRBuilder<>& builder, llvm::Module& module) {
    auto throwFuncCallee = module.getOrInsertFunction("bateman_throw",
        llvm::FunctionType::get(builder.getVoidTy(), {llvm::Type::getInt8PtrTy(module.getContext())}, false));
    auto* throwFunc = llvm::cast<llvm::Function>(throwFuncCallee.getCallee());
    auto* str = builder.CreateGlobalStringPtr(message);
    builder.CreateCall(throwFunc, {str});
    builder.CreateUnreachable();
    return nullptr;
}

llvm::Value* PrintNode::codegen(llvm::IRBuilder<>& builder, llvm::Module& module) {
    auto printFuncCallee = module.getOrInsertFunction("bateman_print",
        llvm::FunctionType::get(builder.getVoidTy(), {builder.getInt32Ty()}, false));
    auto* printFunc = llvm::cast<llvm::Function>(printFuncCallee.getCallee());
    auto* value = expr->codegen(builder, module);
    builder.CreateCall(printFunc, {value});
    return nullptr;
}

llvm::Value* CallNode::codegen(llvm::IRBuilder<>& builder, llvm::Module& module) {
    std::vector<llvm::Value*> args;
    for (auto* arg : this->args) {
        auto* value = arg->codegen(builder, module);
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
    auto* value = expr->codegen(builder, module);
    return builder.CreateRet(value);
}

llvm::Value* InputNode::codegen(llvm::IRBuilder<>& builder, llvm::Module& module) {
    auto* alloc = symbolTable[name];
    if (!alloc) {
        alloc = builder.CreateAlloca(builder.getInt32Ty(), nullptr, name);
        symbolTable[name] = alloc;
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
    auto* alloc = symbolTable[name];
    if (!alloc) throw std::runtime_error("Undeclared variable: " + name);
    return builder.CreateLoad(builder.getInt32Ty(), alloc, name);
}

llvm::Value* BinOpNode::codegen(llvm::IRBuilder<>& builder, llvm::Module& module) {
    auto* leftValue = left->codegen(builder, module);
    auto* rightValue = right->codegen(builder, module);
    if (op == "+") return builder.CreateAdd(leftValue, rightValue);
    if (op == "-") return builder.CreateSub(leftValue, rightValue);
    if (op == "*") return builder.CreateMul(leftValue, rightValue);
    if (op == "/") return builder.CreateSDiv(leftValue, rightValue);
    if (op == "==") return builder.CreateICmpEQ(leftValue, rightValue);
    // TODO: Raise or default for other operators
}