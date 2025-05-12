// Headers shared by both lexer and parser
#ifndef PARSER_SHARED_HPP
#define PARSER_SHARED_HPP

#include <string>
#include <vector>
#include "ast.hpp"

extern std::vector<ASTNode*>* g_root;

extern int yylex();
void yyerror(const char* s);

#endif