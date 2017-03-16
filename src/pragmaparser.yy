%{

/*

Copyright (c) 2017, Lawrence Livermore National Security, LLC.
Produced at the Lawrence Livermore National Laboratory
Written by Markus Schordan (email: schordan1@llnl.gov)
LLNL-CODE-718438
All rights reserved.

This file is part of Backstroke. For details, see
https://github.com/LLNL/backstroke. Please also see the LICENSE file
for our additional BSD notice.

Redistribution of Backstroke and use in source and binary forms, with
or without modification, are permitted provided that the following
conditions are met:

* Redistributions of source code must retain the above copyright
  notice, this list of conditions and the disclaimer below.

* Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the disclaimer (as noted below)
  in the documentation and/or other materials provided with the
  distribution.

* Neither the name of the LLNS/LLNL nor the names of its contributors
  may be used to endorse or promote products derived from this
  software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL
SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING

IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <iostream>
#include "pragmaparser_decls.h"
#include "PragmaMap.h"

using namespace std;

// Parser return type
//#define YYSTYPE Backstroke::Annotation*
Backstroke::Annotation* pragmaAst;

#ifndef NDEBUG
#ifndef YYDEBUG
#define YYDEBUG 1
#endif
#endif

%}

%union {
char* idstring; /* IDENT */
long intval; /* INTEGER */
}

%token PDES
%token EXCLUDE
%token MAP 
%token FORWARD REVERSE COMMIT
%token ORIGINAL EMPTY
%token <idstring> IDENT 
%token <intval> INTEGER

%start start

%%

start        : EXCLUDE
             {
                pragmaAst=new Backstroke::AnnotationExclude();
             }
             | MAP FORWARD ':' IDENT
             | MAP FORWARD '=' ORIGINAL
             {
                pragmaAst=new Backstroke::AnnotationExclude();
             }
             | MAP REVERSE '=' EMPTY
             | MAP REVERSE ':' IDENT
             | MAP COMMIT '=' EMPTY
             | MAP COMMIT ':' IDENT
             ;

%%

void InitializeParser(const std::string& inString) {
    InitializeLexer(inString);
}
void FinishParser() {
    FinishLexer();
}

void
pragmaparsererror(const char* errorMessage)
{
  /* pragmalineno, errorMessage, pragmatext */
  fprintf(stderr,"backstroke pragma syntax error: line %d: %s at %s\n",pragmaparserlineno,errorMessage,pragmaparsertext);
  exit(2);
}

#if 0
//  pragmaparserparse();
int main() {
    int parseok = !yyparse();
    if (!parseok) {
       std::cerr << "parsing failed: " << yylineno << ":"<<yytext<< std::endl;
       return 1;
    }
    return 0;
}
#endif