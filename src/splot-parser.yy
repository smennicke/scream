
%{

// options for Bison
#define YYDEBUG 1
#define YYERROR_VERBOSE 0  // for verbose error messages

#include "Feature.h"
#include "FeatureGroup.h"
#include "FeatureModel.h"
#include "tools.h"
#include "types.h"

extern FeatureModel *fm;
Feature *parent = NULL;
FeatureGroup *group = NULL;
unsigned int last_depth = 0;

// from flex
extern char* splot_yytext;
extern int splot_yylex();
extern int splot_yyerror(const char *msg);
%}

// Bison options
%name-prefix="splot_yy"

%token KEY_ROOT KEY_MANDATORY KEY_OPTIONAL KEY_GROUP KEY_GROUPED
%token LPAR RPAR LDELIM RDELIM ASTERISK TAB COMMA
%token BEGIN_FT END_FT BEGIN_CONS END_CONS
%token OP_OR OP_NOT
%token number ident

// Bison generates a list of all used tokens in file "syntax_splot.h" (for flex)
%token_table
%defines

%union {
  char *str;
  unsigned int value;
  GroupType gtype;
}

%type <value> number
%type <str> ident
%type <str> fname
%type <value> tabs
%type <gtype> card

%%

splot:
  fm
;

fm:
  BEGIN_FT feature_tree END_FT 
  BEGIN_CONS constraints END_CONS
;

feature_tree:
  KEY_ROOT fname 
  { 
    fm = new FeatureModel(new Feature(MANDATORY, $2));
    parent = &fm->getRootFeature();
    last_depth = 1;
  } feature
;

feature:
  group
| tabs KEY_MANDATORY fname
  {
//    fprintf(stderr, "Feature %s: %d (%d)\n", $3, $1, last_depth);
    while ( last_depth - $1 ) {
      group = parent->getGroup(); 
      parent = &parent->getParent();
      if (group != NULL && parent == &group->getParent())
        last_depth--;
      last_depth--;
    }
    parent = &fm->addFeature(MANDATORY, $3, parent);
//    fprintf(stderr, "Added Mandatory %s with parent %s", parent->getName().c_str(), parent->getParent().getName().c_str());
    last_depth++;
  } feature
| tabs KEY_OPTIONAL fname
  {
//    fprintf(stderr, "Feature %s: %d (%d)\n", $3, $1, last_depth);
    while ( last_depth - $1 ) {
      group = parent->getGroup();
      parent = &parent->getParent();
      if (group != NULL && parent == &group->getParent())           
        last_depth--;
      last_depth--;
      scream_status("went up to " + parent->getName());
    }
    parent = &fm->addFeature(OPTIONAL, $3, parent);
    scream_status("optional feature added");
    last_depth++;
  } feature
| tabs KEY_GROUPED fname
  {
//    fprintf(stderr, "Feature %s: %d (%d)\n", $3, $1, last_depth);
    while (last_depth - $1)
    {
      if (group != NULL && parent == &group->getParent())
        last_depth--;
      group = parent->getGroup();
      parent = &parent->getParent();
      last_depth--;
    }
    parent = &fm->addGroupFeature(*group, $3);
    scream_status("Group Feature added");
    last_depth++;
    scream_status("parent set to " + parent->getName());
    scream_status("with parent " + parent->getParent().getName());
  } feature
| /* empty */
;

tabs:
  /* empty */
  { $$ = 0; }
| TAB tabs
  { $$ = 1 + $2; }
;

fname:
  /* empty */
| name LPAR ident RPAR
  { $$ = $3; }
| ident
  { $$ = $1; }
;

name:
  ident name
| /* empty */
;

group:
  tabs KEY_GROUP fname card
  {
//    fprintf(stderr, "Group: %d (%d)\n", $1, last_depth);
//    fprintf(stderr, "Parent: %s (%s)\n", parent->getName().c_str(), parent->getParent().getName().c_str());
    while ( last_depth - $1 ) {
      group = parent->getGroup();
      parent = &parent->getParent();
      if (group != NULL && parent == &group->getParent())
        last_depth--;
      last_depth--;
    }
    group = &fm->addGroup($4, parent);
    scream_status("Group added");
    last_depth++;
  } feature
;

card:
  LDELIM number COMMA number RDELIM
  {
    if ($4 == 1) {
      scream_status("Handling cardinalities as [1,1]");
      $$ = XOR;
    } else {
      scream_status("Handling cardinalities as [1,*]");
      $$ = OR;
    }
  }
| LDELIM number COMMA ASTERISK RDELIM
  {
    switch ($2)
    {
    case 1: /* everything ok */
      break;
    default:
      scream_status("Warning: Found unallowed lower bound");
      break;
    }
    
    scream_status("Handling cardinalities as [1,*]");
    $$ = OR;
  }
;

constraints:
  /* empty */
| ident KEY_GROUPED OP_NOT ident OP_OR ident constraints
  {
    /* implies constraint */
//    fprintf(stderr, "implies constraint: %s\n", $1);
    Feature *f1 = fm->findFeature($4);
    Feature *f2 = fm->findFeature($6);
    if (f1 == NULL)
      splot_yyerror("parse error");
    if (f2 == NULL)
      splot_yyerror("parse error");
    
    f1->requires(*f2);
    fm->addConstraint();
  }
| ident KEY_GROUPED OP_NOT ident OP_OR OP_NOT ident constraints
  {
    /* excludes constraint */
    Feature *f1 = fm->findFeature($4);
    Feature *f2 = fm->findFeature($7);
    if (f1 == NULL)
      splot_yyerror("parse error");
    if (f2 == NULL)
      splot_yyerror("parse error");
    
    f1->excludes(*f2);
    fm->addConstraint();
  }
;
