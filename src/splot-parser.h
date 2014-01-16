/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     KEY_ROOT = 258,
     KEY_MANDATORY = 259,
     KEY_OPTIONAL = 260,
     KEY_GROUP = 261,
     KEY_GROUPED = 262,
     LPAR = 263,
     RPAR = 264,
     LDELIM = 265,
     RDELIM = 266,
     ASTERISK = 267,
     TAB = 268,
     COMMA = 269,
     BEGIN_FT = 270,
     END_FT = 271,
     BEGIN_CONS = 272,
     END_CONS = 273,
     OP_OR = 274,
     OP_NOT = 275,
     number = 276,
     ident = 277
   };
#endif
/* Tokens.  */
#define KEY_ROOT 258
#define KEY_MANDATORY 259
#define KEY_OPTIONAL 260
#define KEY_GROUP 261
#define KEY_GROUPED 262
#define LPAR 263
#define RPAR 264
#define LDELIM 265
#define RDELIM 266
#define ASTERISK 267
#define TAB 268
#define COMMA 269
#define BEGIN_FT 270
#define END_FT 271
#define BEGIN_CONS 272
#define END_CONS 273
#define OP_OR 274
#define OP_NOT 275
#define number 276
#define ident 277




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 2068 of yacc.c  */
#line 38 "splot-parser.yy"

  char *str;
  unsigned int value;
  GroupType gtype;



/* Line 2068 of yacc.c  */
#line 102 "splot-parser.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE splot_yylval;


