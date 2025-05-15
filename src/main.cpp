#include <fstream>
#include <sstream>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/IR/LegacyPassManager.h> // For legacy pass manager
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
    // Enable debug output for parser
    // TODO: this should be a trace debug option
    yydebug = 1;

    // Initialize LLVM
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    llvm::LLVMContext context;
    auto module = std::make_unique<llvm::Module>("bateman", context);

    // Read input source file
    std::ifstream file(argc > 1 ? argv[1] : "input.bateman");
    if (!file.is_open()) {
        std::cerr << "Error: Could not open input file." << std::endl;
        return 1;
    }
    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // Parse input
    lexer.switch_streams(new std::istringstream(source), nullptr);
    if (yyparse() != 0) {
        std::cerr << "Parsing failed due to syntax errors." << std::endl;
        return 1;
    }

    if (!g_root) {
        std::cerr << "Error: No AST generated." << std::endl;
        return 1;
    }

    // Verify AST
    for (size_t i = 0; i < g_root->size(); ++i) {
        if (!(*g_root)[i]) {
            std::cerr << "Error: Null AST node at index " << i << std::endl;
            return 1;
        }
    }

    // Generate LLVM IR
    llvm::IRBuilder<> builder(context);

    /* START OF FUNCTION IMPLEMENTATIONS
        These implement specific functions from the generated object code e.g. bateman_print --> printf
    */

    // Declare printf
    auto printfTy = llvm::FunctionType::get(builder.getInt32Ty(),
                                            {llvm::PointerType::get(builder.getInt8Ty(), 0)},
                                            true);
    module->getOrInsertFunction("printf", printfTy);

    // Define bateman_print: void (i8*)
    auto printTy = llvm::FunctionType::get(builder.getVoidTy(),
        {llvm::PointerType::get(builder.getInt8Ty(), 0)},
        false);
    auto printFn = llvm::Function::Create(printTy, llvm::Function::ExternalLinkage, "bateman_print", module.get());
    auto entryBB = llvm::BasicBlock::Create(context, "entry", printFn);
    llvm::IRBuilder<> printBuilder(entryBB);
    auto strArg = printFn->getArg(0);
    printBuilder.CreateCall(module->getFunction("printf"), {strArg});
    printBuilder.CreateRetVoid();

    /* END OF FUNCTION IMPLEMENTATIONS */

    for (auto* node : *g_root) {
        node->codegen(builder, *module);
    }

    // Set up the target machine
    std::string targetTriple = llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(targetTriple);

    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    if (!target) {
        std::cerr << "Error: " << error << std::endl;
        return 1;
    }

    llvm::TargetOptions opt;
    auto rm = std::optional<llvm::Reloc::Model>(llvm::Reloc::PIC_);
    auto targetMachine = target->createTargetMachine(targetTriple, "generic", "", opt, rm);
    if (!targetMachine) {
        std::cerr << "Error: Failed to create target machine." << std::endl;
        return 1;
    }

    module->setDataLayout(targetMachine->createDataLayout());

    // Emit object code
    std::error_code ec;
    llvm::raw_fd_ostream dest("output.o", ec, llvm::sys::fs::OF_None);
    if (ec) {
        std::cerr << "Error: Could not open output.o: " << ec.message() << std::endl;
        return 1;
    }

    // Use legacy pass manager for code generation
    llvm::legacy::PassManager pass;
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
        std::cerr << "Error: Target machine cannot emit object file." << std::endl;
        return 1;
    }

    pass.run(*module);
    dest.flush();
    dest.close();

    // Optionally print LLVM IR to stdout for debugging
    module->print(llvm::outs(), nullptr);

    // Link object file into executable
    // TODO: Executable name should be passed as an argument
    std::string linkCommand;
    #ifdef _WIN32
        linkCommand = "clang output.o -o resulting_executable.exe";
    #else
        linkCommand = "clang -pie output.o -o resulting_executable";
    #endif

    if (system(linkCommand.c_str()) != 0) {
        std::cerr << "Error: Linking failed." << std::endl;
        return 1;
    }

    std::cout << "Executable generated: " << (argc > 2 ? argv[2] : "resulting_executable") << std::endl;
    return 0;
}