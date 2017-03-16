#ifndef BACKSTROKE_UTILITY_H
#define BACKSTROKE_UTILITY_H

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

#include <list>
#include <string>
#include <utility>
#include <set>
#include <vector>
#include "SgNodeHelper.h"

// only required as long as Unparser_Opt is used
#include "unparser.h"

namespace Backstroke {
  namespace Utility {
    typedef size_t LineNr;
    typedef size_t ColNr;
    void printRoseInfo(SgProject* project);

    // compute set of all statements (including root node) of subtree.
    std::set<SgNode*> subTreeNodes(SgNode* node);

    // computes number of files present in a project
    size_t numberOfFiles(SgProject* project);

    // returns a reference to a list of pointers to all files in a project
    const SgFilePtrList& getFilePtrList(SgProject* project);

    // returns the single source file in a project.  if there is more
    // than one file or no file, it returns 0.
    SgFile* singleSourceFile(SgProject* project);

    // returns the file name of a single source file in a project.  if
    // there is more than one file or no file, it returns an empty
    // string
    std::string singleSourceFileName(SgProject* project);

    SgClassDeclaration* isSpecialization(SgNode* node);
    /* determines whether a node is any of: 
       SgTemplateInstantiationDecl
       SgTemplateInstantiationDefn
       SgTemplateInstantiationFunctionDecl
       SgTemplateInstantiationMemberFunctionDecl
       SgTemplateInstantiationTypedefDeclaration
       SgTemplateInstantiationDirectiveStatement
    */
    bool isTemplateInstantiationNode(SgNode* node);

    /* collects all pragmas with name 'pragmaName' and creates a list of all 
       pragma strings (with stripped off prefix) and the associated SgNode. */
    //typedef std::list<std::pair<std::string,SgNode*> > PragmaList;
    SgNodeHelper::PragmaList collectPragmaLines(std::string pragmaName,SgNode* root);

    // replace in string 'str' each string 'from' with string 'to'.
    void replaceString(std::string& str, const std::string& from, const std::string& to);

    // checks whether prefix 'prefix' is a prefix in string 's'.
    bool isPrefix(const std::string& prefix, const std::string& s);

    // checks whether 'elem' is the last child (in traversal order) of node 'parent'.
    bool isLastChildOf(SgNode* elem, SgNode* parent);

    /* return the verbatim pragma string as it is found in the source code
       this string includes the leading "#pragma".
    */
    std::string getPragmaDeclarationString(SgPragmaDeclaration* pragmaDecl);

    /* marks node 'node' to be unparsed. */
    void markNodeToBeUnparsed(SgLocatedNode* node);

    /* marks node 'node' to be unparsed. */
    bool isMarkedNodeToBeUnparsed(SgNode* node);

    /* computes the list of all function definitions in the provided AST */
    std::list<SgFunctionDefinition*> listOfFunctionDefinitions(SgNode* node);

    void writeStringToFile(std::string data, std::string fileName);
    
    /* finds the first non-white space location in the string (beginning at the start of the string)
       If only white space are found then the return value equals data.size() */
    size_t findNonWhiteSpacePos(std::string data);
    
    // finds all occurences of substring sub in string str and returns a vector with starting positions
    std::vector<size_t> findSubstringOccurences(std::string sub, std::string str);
    
    struct LineCol {
      LineCol();
      LineCol(LineNr line, ColNr col);
      std::string toString();
      LineNr line;
      ColNr col;
      bool valid;
    };

    class LineColName {
    public:
      LineColName(LineNr line, ColNr col, std::string name, bool isNamespaceName=false, bool isGlobalNamespaceName=false);
      bool isNamespaceName();
      bool isGlobalNamespaceName();
      LineCol getLineCol();
      std::string getName();
      std::string toString();
      std::string getFullName();
      size_t getFullNameLength();
      size_t getNameLength();
    private:
      LineCol _lineCol;
      std::string _name;
      bool _isNamespaceName;
      bool _isGlobalNamespaceName;
    };
    
    class SplicedName : public std::vector<LineColName> {
    public:
      std::string toString();
      std::string getFullName();
      size_t getFullNameLength();
    };
    
    // determines namespace names and name starting at position col
    SplicedName spliceName(std::string s, LineNr line, ColNr colStart);
    typedef std::vector<SgExpression*> ArrayDimensionExpressions;
    /* returns a vector of all dimensions (ArrayDimensionExpressions)
       of a possibly nested array type; the expressions in ArrayDimensionExpressions
       are a copy of the AST expressions and need to be deleted by the
       caller. This can be done by the function removeArrayDimensions. */
    ArrayDimensionExpressions getArrayDimensionExpressions(const SgArrayType& arrtype);
    void deleteArrayDimensionExpressions(ArrayDimensionExpressions arrayDimensionExpressions);

    size_t getArrayElementCount(SgArrayType* arrayType);

    // backend support functions
    Unparser_Opt defaultRoseOptions();
    void initializeSgUnparseInfo(SgUnparse_Info* inheritedAttributeInfoPointer);
    std::string unparseTypeToString(SgType* type, SgNode* referenceNodeForNameQualification=0);
    std::string unparseExprToString(SgExpression* type);

    void showClassDefinition(SgClassDefinition* classDef);
    void showGlobalDeclarations(SgNode* node);
    bool isInsideNamespace(SgDeclarationStatement* declStmt);

    bool checkRoseVersionNumber(const std::string &need);
  } // namespace Utility
} // namespace Backstroke

#endif
