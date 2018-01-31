#ifndef CLASS_INFO_H
#define CLASS_INFO_H

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

#include "CommandLineOptions.h"

namespace Backstroke {
  // this class stores all required information for generating default operators
  class ClassInfo {
  public:
    ClassInfo();
    ~ClassInfo();
    void initialize(SgClassDefinition*);
  private:
    void initializeDataMemberProperties();
  public:
    int keyWordLength();
    // returns a class name (also for anonyous structs/classes/unions!)
    std::string className();
    bool hasDefaultConstructor();
    void setDefaultConstructor();
    bool hasDefaultCopyConstructor();
    void setDefaultCopyConstructor();
    bool hasDefaultCopyAssignOperator();
    void setDefaultCopyAssignOperator();
    bool hasAnyCopyAssignOperator();
    void setAnyCopyAssignOperator();
    bool hasDefaultMoveAssignOperator();
    void setDefaultMoveAssignOperator();
    void setDefaultProtectedOrPrivateDestructor();
    bool hasDefaultProtectedOrPrivateDestructor();
    bool hasConstDataMember();
    bool hasCLinkage();
    bool isAnonymous();
    bool isClass();
    bool isStruct();
    bool isUnion();
    // determines all types of data members inside unions
    static std::set<SgType*> dataMemberTypesInsideUnions(SgNode* node);
    static bool isUnionDeclaration(SgNode*);
    bool isInsideUnion();
    static bool isInsideUnion(SgClassDeclaration* classDecl);
    static bool isRefTypeOfClass(SgType* argType, SgClassDefinition* classDef);
    static bool isConstRefTypeOfClass(SgType* argType, SgClassDefinition* classDef);
    std::string implConstructor();
    std::string implCopyConstructor();
    std::string implCopyAssignOperator();
    std::string implAllDefaultOperators(SgClassDefinition* classDef, CommandLineOptions* clo, std::set<SgType*>& dataMemberTypesInsideUnions);
    int injectionLine();
    int injectionCol();
    typedef std::vector< std::pair< SgNode*, std::string > > DataMemberPointers;
    std::list<SgVariableDeclaration*> getDataMembers();
    size_t numberOfDataMembers();
    void setRTSSAccess(std::string s);
    std::string getRTSSAccess();
    bool isPThreadRelevantClass();
    std::vector<SgInitializedNamePtrList*> getConstructorInitializerLists();
    std::string name();
    typedef std::list<SgVariableDeclaration*> BitFieldDeclarationList;
    BitFieldDeclarationList getBitFieldDeclarationList();
    std::size_t numBitFields();
    SgAccessModifier getFirstDataMemberAccessModifier();
    bool generatedDefaultAssignOp();
  private:
    void determineBitFields();
    std::string reversibleCopyCode();
    SgVariableDeclaration* firstDataMember();
    BitFieldDeclarationList _bitFieldVarDecls;
    bool _defaultConstructor;
    bool _defaultCopyConstructor;
    bool _anyCopyAssignOperator;
    bool _defaultCopyAssignOperator;
    bool _defaultMoveAssignOperator;
    bool _defaultProtectedOrPrivateDestructor;
    SgClassDeclaration* _classDecl;
    SgClassDefinition* _classDef;
    std::string _className;
    std::string _rtssAccess;
    bool _hasConstDataMember;
    bool _hasCLinkage; // obsolete
    bool _generatedDefaultAssignOp=false;
  };

} // end of namespace Backstroke

#endif
