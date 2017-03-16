#ifndef _parser_decls_h
#define _parser_decls_h

#include <string>

// These are global declarations needed for the lexer and parser files.

// Front End specific classes
class CCompiler;
void InitializeParser(const std::string& inString);
void FinishParser();

void InitializeLexer(const std::string& tokenizeString);
void FinishLexer();

extern char* pragmaparsertext;
extern int pragmaparserlineno;
extern bool pragmaparserdeprecated;
int pragmaparserlex();
int pragmaparserparse();
void pragmaparsererror(const char*);

#endif
