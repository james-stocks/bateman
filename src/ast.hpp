#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <vector>
#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual llvm::Value* codegen(llvm::IRBuilder<>& builder, llvm::Module& module) = 0;
};

class VarDeclNode : public ASTNode {
    std::string name;
    ASTNode* init;
public:
    VarDeclNode(const std::string& name, ASTNode* init = nullptr) : name(name), init(init) {}
    llvm::Value* codegen(llvm::IRBuilder<>& builder, llvm::Module& module) override;
};

class FuncDeclNode : public ASTNode {
    std::string name;
    std::vector<std::string> params;
    std::vector<ASTNode*> body;
public:
    FuncDeclNode(const std::string& name, const std::vector<std::string>& params, const std::vector<ASTNode*>& body)
        : name(name), params(params), body(body) {}
    llvm::Value* codegen(llvm::IRBuilder<>& builder, llvm::Module& module) override;
};

class RaiseNode : public ASTNode {
    std::string message;
public:
    RaiseNode(const std::string& msg) : message(msg) {}
    llvm::Value* codegen(llvm::IRBuilder<>& builder, llvm::Module& module) override;
};

class PrintNode : public ASTNode {
    ASTNode* expr;
public:
    PrintNode(ASTNode* expr) : expr(expr) {}
    llvm::Value* codegen(llvm::IRBuilder<>& builder, llvm::Module& module) override;
};

class CallNode : public ASTNode {
    std::string name;
    std::vector<ASTNode*> args;
public:
    CallNode(const std::string& name, const std::vector<ASTNode*>& args) : name(name), args(args) {}
    llvm::Value* codegen(llvm::IRBuilder<>& builder, llvm::Module& module) override;
};

class ReturnNode : public ASTNode {
    ASTNode* expr;
public:
    ReturnNode(ASTNode* expr) : expr(expr) {}
    llvm::Value* codegen(llvm::IRBuilder<>& builder, llvm::Module& module) override;
};

class InputNode : public ASTNode {
    std::string name;
public:
    InputNode(const std::string& name) : name(name) {}
    llvm::Value* codegen(llvm::IRBuilder<>& builder, llvm::Module& module) override;
};

class NumberNode : public ASTNode {
    int value;
public:
    NumberNode(int val) : value(val) {}
    llvm::Value* codegen(llvm::IRBuilder<>& builder, llvm::Module& module) override;
};

class StringNode : public ASTNode {
    std::string value;
public:
    StringNode(const std::string& val) : value(val) {}
    llvm::Value* codegen(llvm::IRBuilder<>& builder, llvm::Module& module) override;
};

class IdentNode : public ASTNode {
    std::string name;
public:
    IdentNode(const std::string& name) : name(name) {}
    llvm::Value* codegen(llvm::IRBuilder<>& builder, llvm::Module& module) override;
};

class BinOpNode : public ASTNode {
    ASTNode* left;
    std::string op;
    ASTNode* right;
public:
    BinOpNode(ASTNode* left, const std::string& op, ASTNode* right) : left(left), op(op), right(right) {}
    llvm::Value* codegen(llvm::IRBuilder<>& builder, llvm::Module& module) override;
};

#endif