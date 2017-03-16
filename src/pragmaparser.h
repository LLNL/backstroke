/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

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

#ifndef YY_PRAGMAPARSER_Y_TAB_H_INCLUDED
# define YY_PRAGMAPARSER_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int pragmaparserdebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    PDES = 258,
    EXCLUDE = 259,
    MAP = 260,
    FORWARD = 261,
    REVERSE = 262,
    COMMIT = 263,
    ORIGINAL = 264,
    EMPTY = 265,
    IDENT = 266,
    INTEGER = 267
  };
#endif
/* Tokens.  */
#define PDES 258
#define EXCLUDE 259
#define MAP 260
#define FORWARD 261
#define REVERSE 262
#define COMMIT 263
#define ORIGINAL 264
#define EMPTY 265
#define IDENT 266
#define INTEGER 267

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 70 "./pragmaparser.yy" /* yacc.c:1909  */

char* idstring; /* IDENT */
long intval; /* INTEGER */

#line 83 "y.tab.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE pragmaparserlval;

int pragmaparserparse (void);

#endif /* !YY_PRAGMAPARSER_Y_TAB_H_INCLUDED  */
