#include <fstream>
#include <sstream>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include "globals.hpp"
#include "parser.hpp"
#include <FlexLexer.h>

extern int yydebug;

std::vector<ASTNode*>* g_root = nullptr;

// Implement flex's yyerror
void yyerror(const char* s) {
    std::cerr << "Parse error: " << s << std::endl;
}

// Wrap flex's yylex function, bison expects a standalone function 
yyFlexLexer lexer;
int yylex() {
    return lexer.yylex();
}

int main(int argc, char* argv[]) {
    // TODO: Make this a trace option
    yydebug = 1;
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::LLVMContext context;
    auto module = std::make_unique<llvm::Module>("bateman", context);
    llvm::IRBuilder<> builder(context);

    std::ifstream file(argc > 1 ? argv[1] : "input.bateman");
    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    
    yyFlexLexer lexer;
    lexer.switch_streams(new std::istringstream(source), nullptr);
    if (yyparse() != 0) {  // Check if parsing failed
        std::cerr << "Parsing failed due to syntax errors." << std::endl;
        return 1;
    }

    if (!g_root) {  // Check if the AST is null
        std::cerr << "Error: No AST generated." << std::endl;
        return 1;
    }

    for (auto* node : *g_root) {
        node->codegen(builder, *module);
    }

    module->print(llvm::outs(), nullptr);
    return 0;
}