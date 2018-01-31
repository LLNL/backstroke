#ifndef TRANSFORMATION_SEQUENCE_H
#define TRANSFORMATION_SEQUENCE_H

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
#include <utility>
#include <string>
#include <string>
#include <set>
#include <map>
#include <list>
#include "sage3basic.h"
#include "PragmaMap.h"
#include "CommandLineOptions.h"
#include "ClassInfo.h"
#include "EditSequence.h"

class SgNode;
class SgExpression;

namespace Backstroke {
  enum TransformationOperation { TRANSOP_ASSIGNMENT, TRANSOP_MEM_ALLOC, TRANSOP_MEM_ALLOC_ARRAY, TRANSOP_MEM_DELETE, TRANSOP_MEM_DELETE_ARRAY, TRANSOP_CLASS_DEF, TRANSOP_DECL_BITFIELD, TRANSOP_FUNCALL_OP_DELETE, TRANSOP_FUNCALL_OP_NEW, TRANSOP_MEMBER_FUNCTION_DECL, TRANSOP_NUM };

  class TransformationSequence;
  class TransformationStatistics {
  public:
       TransformationStatistics();
    void init();
    void increment(TransformationOperation transOp);
    void decrement(TransformationOperation transOp);
    std::string toString();
    std::string toCsvString();
    int generatedDefaultAssignOpNum=0;
  private:
    size_t operatorsCount[TRANSOP_NUM];
  };

  enum ConstructorInitListType { INIT_DataMember, INIT_DELEGATION_CONSTRUCTOR };
  class ConstructorInitListMemberInfo {
  public:
  ConstructorInitListMemberInfo(ConstructorInitListType ilt):_constructorInitListType(ilt){}
    bool isDataMember() { return _constructorInitListType==INIT_DataMember;}
    bool isDelegationConstructor() { return _constructorInitListType==INIT_DELEGATION_CONSTRUCTOR;}
    std::string varName() { return _varName; }
  private:
    ConstructorInitListType _constructorInitListType;
    std::string _varName;
  };

  struct LineColInfo {
    int startLine;
    int startCol;
    int endLine;
    int endCol;
    std::string toString() { 
      std::stringstream ss;
      ss<<startLine<<","<<startCol<<":"<<endLine<<","<<endCol;
      return ss.str();
    }
  };

  class TransformationSequence : protected AstPrePostProcessing {
  public:
    TransformationSequence();
    virtual ~TransformationSequence();
    static LineColInfo lineColInfo(SgLocatedNode* locNode);
    void reset();
    void create(SgNode*);
    void applyToFile(OriginalFile& sourceFile);
    void setRTSSVariableName(std::string name) { _rtssVariableName=name; }
    void setRTSSAccessOperatorArrow();
    void setRTSSAccessOperatorDot();
    void setRTSSNamespaceName(std::string name);
    void setPragmaMap(PragmaMap* p) { _pragmaMap=p; }
    std::string computeSourceFragment(int startLine, int startCol, int endLine, int endCol, OriginalFile& originalFile);
    std::string editSequenceToString() { return _editSequence.toString(); }
    EditSequence& editSequence() { return _editSequence; }
    TransformationStatistics getTransformationStatistics() { return _transformationStatistics; }
    void setCommandLineOptions(CommandLineOptions* clo) { _commandLineOptions=clo; }
    ClassInfo determineClassInfo(SgClassDefinition* classDef);

  protected:
    virtual void preOrderVisit(SgNode *astNode);
    virtual void postOrderVisit(SgNode *astNode);

  private:
    Backstroke::Utility::LineCol findNextChar(int startLine,int startCol,char c, OriginalFile const& originalFile);
    PragmaMap* _pragmaMap;
    typedef std::list<std::pair<TransformationOperation,SgNode*> > TransformationSequenceList;
    TransformationSequenceList transformationSequenceList;
    static bool isPointerType(SgType* type);
    static bool isReferenceType(SgType* type);
    static bool isLocalVariable(SgExpression* exp);
    static bool isClassType(SgType* type);

  public:
    static bool isArrayType(SgType* type);

  private:
    static bool isEnumType(SgType* type);
    bool _showTransformationTrace;
    SgNode* isInsideCompilerGeneratedTemplateInstantiation(SgNode* node);

    std::string rtssFunctionCallCode();
    std::string _rtssVariableName;
    std::string _rtssAccessOperator;
    std::string _rtssNamespaceName;
    int _rtssAccessMode;

    // returns the base type of a reference. If type is not a reference it returns 0.
    SgType* getReferenceBaseType(SgType* type);
  public:
    std::string generateRTSSAccess();
    // returns the length of the keyword for class(=5)/struct(=6)/union(=5).
  private:
    std::string generateAVPushPtr(SgExpression* exp);
    std::string generateAVPush(SgExpression* exp);
    std::string generateEnumPush(SgExpression* exp);
    std::string generateBuiltInTypeRefPush(SgExpression* exp, SgType* refBaseType);
    SgExpression* adjustCompilerGeneratedExpr(SgExpression* expr);

    std::list<SgType*> _assignedUserDefinedTypes;
    EditSequence _editSequence;
    TransformationStatistics _transformationStatistics;
    bool _eliminateGlobalDecls;
    CommandLineOptions* _commandLineOptions;
    std::set<SgType*> _dataMemberTypesInsideUnions;
  };
};

#endif
