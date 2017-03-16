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
#include <assert.h>
#include <stdlib.h>

#include <string>
#include <cstring>

#include "pragmaparser_decls.h"
#include "pragmaparser.h"

%}

%option yylineno
%option noyywrap
%option never-interactive
%option outfile="lex.yy.c"

digit           [0-9]
letter          [a-zA-Z_]

%%
"reversible" { return PDES;}
"pdes" { return PDES;}
"exclude" { return EXCLUDE;}
"map" { return MAP;}
"forward" { return FORWARD;}
"reverse" { return REVERSE;}
"commit" { return COMMIT;}
"empty" { return EMPTY;}
"original" { return ORIGINAL;}
"="             { return '='; }
":"             { return ':'; }
","             { return ','; }
";"             { return ';'; }
{letter}({letter}|{digit}|"+")*  { pragmaparserlval.idstring = strdup(pragmaparsertext); return IDENT; }
[ \t\r]         ; /*white space*/
\n              ; /* we are using #option yylineno*/
{digit}{digit}* { pragmaparserlval.intval = atoi(pragmaparsertext); return INTEGER; }
.               { printf("ERROR 1: Lexical error! : <%s>\n",pragmaparsertext); exit(1);}
%%

YY_BUFFER_STATE gInputBuffer;

void
InitializeLexer(const std::string& tokenizeString)
{
    gInputBuffer=yy_scan_string(tokenizeString.c_str());
}

void
FinishLexer()
{
    yy_delete_buffer(gInputBuffer);
}

/*
\"[^"\n]**["\n]    { int slen=strlen(pragmaparsertext+1); char* snew=(char*)malloc(slen-1+1);strncpy(snew,pragmaparsertext+1,slen-1);snew[slen-1]=0; pragmaparserlval.string = snew; return STRING; }
*/
