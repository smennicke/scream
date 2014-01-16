/* flex options */
%option outfile="lex.yy.c"
%option prefix="splot_yy"
%option noyywrap
%option yylineno
%option nodefault

%{
#include <cstdio>
#include <string>
#include "tools.h"
#include "types.h"

#include "splot-parser.h"

extern std::string filename;
extern int splot_yyerror(const char *msg);
%}

whitespace     [\n\r ]
identifier     [^,;:()\t \n\r\{\}=~\[\]\*]+
number         [0-9]+

%%

"<feature_tree>"      { return BEGIN_FT; }
"</feature_tree>"     { return END_FT; }

"<constraints>"       { return BEGIN_CONS; }
"</constraints>"      { return END_CONS; }

":r"                  { return KEY_ROOT; }
"r:"                  { return KEY_ROOT; }

":m"                  { return KEY_MANDATORY; }
":o"                  { return KEY_OPTIONAL; }

":g"                  { return KEY_GROUP; }
":"                   { return KEY_GROUPED; }

"("                   { return LPAR; }
")"                   { return RPAR; }

"["                   { return LDELIM; }
"]"                   { return RDELIM; }

"*"                   { return ASTERISK; }
","                   { return COMMA; }

"or"                  { return OP_OR; }
"~"                   { return OP_NOT; }

\t                    { return TAB; }

{number}              { splot_yylval.value = atoi(splot_yytext); return number; }
{identifier}          { splot_yylval.str = strdup(splot_yytext); return ident;  }

{whitespace}          { /* ignore */ }

.                     { splot_yyerror("lexical error"); }

%%

int splot_yyerror(const char *msg) {
  fprintf(stderr, "parse error in %s:%d: %s - token last read '%s'\n",
    filename.c_str(), splot_yylineno, msg, splot_yytext);
  return 1;
}
