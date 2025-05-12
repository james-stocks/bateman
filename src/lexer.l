%{
#include "shared_headers.hpp"
#include "parser.hpp"
#include <iostream>
%}

%option c++
%option noyywrap
%x KEYWORD_I KEYWORD_LETS KEYWORD_DO KEYWORD_THEIR KEYWORD_IM KEYWORD_FEED KEYWORD_COOL KEYWORD_CAN

%%

"I"                     { BEGIN(KEYWORD_I); yylval.str = new std::string("I"); }
<KEYWORD_I>" have to return some videotapes" { BEGIN(INITIAL); delete yylval.str; return FUNCDECL; }
<KEYWORD_I>" live in the American Gardens Building" { BEGIN(INITIAL); delete yylval.str; return VARDECL; }
<KEYWORD_I>" simply am not there" { BEGIN(INITIAL); delete yylval.str; return RETURN; }
<KEYWORD_I>[ \t\n]      { BEGIN(INITIAL); return IDENT; }

"Let's"                 { BEGIN(KEYWORD_LETS); }
<KEYWORD_LETS>" see Paul Allen's card" { BEGIN(INITIAL); return PRINT; }
<KEYWORD_LETS>[ \t\n]   { BEGIN(INITIAL); yylval.str = new std::string("Let's"); return IDENT; }

"Do"                    { BEGIN(KEYWORD_DO); }
<KEYWORD_DO>" you like Huey Lewis and the News?" { BEGIN(INITIAL); return IF; }
<KEYWORD_DO>[ \t\n]     { BEGIN(INITIAL); yylval.str = new std::string("Do"); return IDENT; }

"Their"                 { BEGIN(KEYWORD_THEIR); }
<KEYWORD_THEIR>" early work was a little too new wave for my taste" { BEGIN(INITIAL); return ELSE; }
<KEYWORD_THEIR>[ \t\n]  { BEGIN(INITIAL); yylval.str = new std::string("Their"); return IDENT; }

"I'm"                   { BEGIN(KEYWORD_IM); }
<KEYWORD_IM>" into murders and executions, mostly" { BEGIN(INITIAL); return WHILE; }
<KEYWORD_IM>[ \t\n]     { BEGIN(INITIAL); yylval.str = new std::string("I'm"); return IDENT; }

"Feed"                  { BEGIN(KEYWORD_FEED); }
<KEYWORD_FEED>" me a stray cat" { BEGIN(INITIAL); return INPUT; }
<KEYWORD_FEED>[ \t\n]   { BEGIN(INITIAL); yylval.str = new std::string("Feed"); return IDENT; }

"Cool"                  { BEGIN(KEYWORD_COOL); }
<KEYWORD_COOL>" it with the anti-Semitic remarks".* { BEGIN(INITIAL); /* ignore comment */ }
<KEYWORD_COOL>[ \t\n]   { BEGIN(INITIAL); yylval.str = new std::string("Cool"); return IDENT; }

"Can"                   { BEGIN(KEYWORD_CAN); }
<KEYWORD_CAN>" you take two tonight?" { BEGIN(INITIAL); return CALL; }
<KEYWORD_CAN>[ \t\n]    { BEGIN(INITIAL); yylval.str = new std::string("Can"); return IDENT; }

"Hey, Paul!"            { return RAISE; }
"This is not an exit"   { return BREAK; }

[a-zA-Z_][a-zA-Z0-9_]* { yylval.str = new std::string(yytext); return IDENT; }
[0-9]+                  { yylval.num = atoi(yytext); return NUMBER; }
\"[^\"]*\"              { yylval.str = new std::string(yytext); return STRING; }
"=="                    { yylval.str = new std::string(yytext); return OPERATOR; }
"+"|"-"|"*"|"/"         { yylval.str = new std::string(yytext); return OPERATOR; }
"="                     { return EQUALS; }
"("                     { return LPAREN; }
")"                     { return RPAREN; }
"{"                     { return LBRACE; }
"}"                     { return RBRACE; }
","                     { return COMMA; }
";"                     { return SEMICOLON; }
[ \t\n]                 { /* ignore whitespace */ }
.                       { std::cerr << "Token: " << yytext << std::endl; yyerror(("Invalid character: " + std::string(yytext)).c_str()); return 0; }

%%