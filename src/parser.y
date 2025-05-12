%debug
%{
#include "shared_headers.hpp"
#include "globals.hpp"
#include <iostream>
%}

%union {
    std::string* str;
    int num;
    ASTNode* node;
    std::vector<std::string>* params;
    std::vector<ASTNode*>* stmt_list;
}

%token FUNCDECL RAISE VARDECL PRINT IF ELSE WHILE BREAK RETURN CALL INPUT EQUALS
%token LPAREN RPAREN LBRACE RBRACE COMMA SEMICOLON
%token <str> IDENT STRING OPERATOR
%token <num> NUMBER

%left OPERATOR  // Resolve shift/reduce conflict for expressions

%type <node> statement func_decl var_decl raise expr call print return_stmt input_stmt
%type <params> param_list
%type <stmt_list> statement_list expr_list

%%
program: statement_list {
    g_root = $1;
    std::cerr << "Parsed program successfully." << std::endl;    
}
opt_semicolon: /* empty */
    | SEMICOLON
statement_list: statement { $$ = new std::vector<ASTNode*>(); $$->push_back($1); }
            | statement_list statement { $1->push_back($2); $$ = $1; }
statement: func_decl | var_decl | raise | print | call | return_stmt | input_stmt
func_decl: FUNCDECL IDENT LPAREN param_list RPAREN LBRACE statement_list RBRACE
           { $$ = new FuncDeclNode(*$2, *$4, *$7); delete $2; delete $4; delete $7; }
var_decl: VARDECL IDENT SEMICOLON { $$ = new VarDeclNode(*$2); delete $2; }
        | VARDECL IDENT EQUALS expr SEMICOLON { $$ = new VarDeclNode(*$2, $4); delete $2; }
raise: RAISE STRING SEMICOLON { $$ = new RaiseNode(*$2); delete $2; }
print: PRINT expr opt_semicolon { $$ = new PrintNode($2); }
call: CALL IDENT LPAREN expr_list RPAREN SEMICOLON { $$ = new CallNode(*$2, *$4); delete $2; delete $4; }
return_stmt: RETURN expr SEMICOLON { $$ = new ReturnNode($2); }
input_stmt: INPUT IDENT SEMICOLON { $$ = new InputNode(*$2); delete $2; }
param_list: /* empty */ { $$ = new std::vector<std::string>(); }
          | IDENT { $$ = new std::vector<std::string>(); $$->push_back(*$1); delete $1; }
          | param_list COMMA IDENT { $1->push_back(*$3); $$ = $1; delete $3; }
expr_list: /* empty */ { $$ = new std::vector<ASTNode*>(); }
         | expr { $$ = new std::vector<ASTNode*>(); $$->push_back($1); }
         | expr_list COMMA expr { $1->push_back($3); $$ = $1; }
expr: NUMBER { $$ = new NumberNode($1); }
    | STRING { $$ = new StringNode(*$1); delete $1; }
    | IDENT { $$ = new IdentNode(*$1); delete $1; }
    | expr OPERATOR expr { $$ = new BinOpNode($1, *$2, $3); delete $2; }
%%