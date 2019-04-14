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

#include <algorithm>
#include "sage3basic.h"
#include "TransformationSequence.h"
#include "SgNodeHelper.h"
#include "CommandLineOptions.h"
#include "RoseAst.h"
#include "Utility.h"
#include "AstTerm.h"

using namespace std;

Backstroke::LineColInfo Backstroke::TransformationSequence::lineColInfo(SgLocatedNode* locNode) {
  int edgCorr=-1;
  Backstroke::LineColInfo li;
  li.startLine=locNode->get_startOfConstruct()->get_raw_line()+edgCorr;
  li.startCol=locNode->get_startOfConstruct()->get_raw_col()+edgCorr;
  li.endLine=locNode->get_endOfConstruct()->get_raw_line()+edgCorr;
  li.endCol=locNode->get_endOfConstruct()->get_raw_col()+edgCorr;
  return li;
}

Backstroke::ClassInfo Backstroke::TransformationSequence::determineClassInfo(SgClassDefinition* classDef) {
  ClassInfo ci;
  ci.initialize(classDef);
  return ci;
}

Backstroke::TransformationSequence::TransformationSequence():
                                                             _rtssVariableName("rts"),
                                                             _rtssAccessOperator("."),
                                                             _commandLineOptions(0)
{
}

Backstroke::TransformationSequence::~TransformationSequence() {
}

void Backstroke::TransformationSequence::reset() {
  transformationSequenceList.clear();
}
 
void Backstroke::TransformationSequence::preOrderVisit(SgNode *astNode) {
}

void Backstroke::TransformationSequence::postOrderVisit(SgNode *astNode) {
  
  //cout<<"DEBUG: visit:"<<astNode->class_name()<<endl;

  if(isInsideCompilerGeneratedTemplateInstantiation(astNode)) {
    return;
  } else if(SgClassDefinition* classDef=isSgClassDefinition(astNode)) {
    transformationSequenceList.push_back(make_pair(TRANSOP_CLASS_DEF,classDef));
  } else if(_pragmaMap->isExcludedNode(astNode)) {
    // detected backstroke directive that excludes nodes of being transformed
    return;
  } else if(isSgAssignOp(astNode)) {
    SgNode* lhs;
    lhs=SgNodeHelper::getLhs(astNode);
    transformationSequenceList.push_back(make_pair(TRANSOP_ASSIGNMENT,lhs));
  } else if(isSgPlusAssignOp(astNode) // +=
            || isSgMinusAssignOp(astNode) // -=
            || isSgMultAssignOp(astNode) // *=
            || isSgDivAssignOp(astNode) // /=
            || isSgModAssignOp(astNode) // %=
            || isSgAndAssignOp(astNode) // &=
            || isSgIorAssignOp(astNode) // |=
            || isSgXorAssignOp(astNode) // ^=
            || isSgLshiftAssignOp(astNode) // <<=
            || isSgRshiftAssignOp(astNode) // >>=
            ) {
    SgNode* lhs=SgNodeHelper::getLhs(astNode);
    transformationSequenceList.push_back(make_pair(TRANSOP_ASSIGNMENT,lhs));
  } else if(isSgPlusPlusOp(astNode) || isSgMinusMinusOp(astNode)) {
    SgNode* operand=SgNodeHelper::getUnaryOpChild(astNode);
    transformationSequenceList.push_back(make_pair(TRANSOP_ASSIGNMENT,operand));
  } else if(SgNewExp* newExp=isSgNewExp(astNode)) {
    // only generate an allocation transformation operation if it is *not* a placement-new operator
    if(newExp->get_placement_args ()==0) {
      if(isSgArrayType(newExp->get_specified_type())) {
        transformationSequenceList.push_back(make_pair(TRANSOP_MEM_ALLOC_ARRAY,newExp));
      } else {
        transformationSequenceList.push_back(make_pair(TRANSOP_MEM_ALLOC,newExp));
      }
    } else {
      if(_commandLineOptions->optionDebug()) cout<<"DEBUG: detected placement new operator."<<endl;
    }
  } else if(SgDeleteExp* deleteExp=isSgDeleteExp(astNode)) {
    //SgNode* operand=SgNodeHelper::getFirstChild(astNode);
    if(deleteExp->get_is_array()) {
      transformationSequenceList.push_back(make_pair(TRANSOP_MEM_DELETE_ARRAY,astNode));
    } else {
      transformationSequenceList.push_back(make_pair(TRANSOP_MEM_DELETE,astNode));
    }
  } else if(SgFunctionCallExp* funCallExp=isSgFunctionCallExp(astNode)) {
    if(SgFunctionSymbol* funSym=funCallExp->getAssociatedFunctionSymbol()) {
      string funName=string(funSym->get_name());
      // support for selected functions by name
      if(funName=="operator delete") {
        //cout<<"funName:"<<funName<<" :: ";
        //SgExprListExp* exprListExp=funCallExp->get_args();
        // TODO: check: operator new(size), operator delete (x), malloc, free
        //cout<<"FunCallExp:"<<funCallExp->unparseToString()<<endl;
        transformationSequenceList.push_back(make_pair(TRANSOP_FUNCALL_OP_DELETE,astNode));
      } else if(funName=="operator new") {
        //cout<<"funName"<<funName<<" :: "<<"FunCallExp:"<<funCallExp->unparseToString()<<endl;
        transformationSequenceList.push_back(make_pair(TRANSOP_FUNCALL_OP_NEW,astNode));
      }
    }
  }

#if 0
  else if(SgMemberFunctionDeclaration* memFunDecl=isSgMemberFunctionDeclaration(astNode)) {
    // allows support of selected member functions
    SgName funName=memFunDecl->get_qualified_name();
    if(funName=="xxx") {
      transformationSequenceList.push_back(make_pair(TRANSOP_MEMBER_FUNCTION_DECL,astNode));
    }
  }
#endif

}

// computes the list of nodes for which the bs-memory-mod transformation must be applied
void Backstroke::TransformationSequence::create(SgNode* node) {
  if(SgFile* file=isSgFile(node)) {
    traverseWithinFile(file);
    if(_commandLineOptions->optionUnion()==false) {
      _dataMemberTypesInsideUnions=ClassInfo::dataMemberTypesInsideUnions(file);
      if(_commandLineOptions->optionDebug()) cout<<"DEBUG: number of data members inside unions: "<<_dataMemberTypesInsideUnions.size()<<endl;
    }
  } else {
    traverse(node);
  }
}

string Backstroke::TransformationSequence::generateRTSSAccess() {
  return _rtssVariableName+_rtssAccessOperator;
}

string Backstroke::TransformationSequence::generateAVPushPtr(SgExpression* exp) {
  SgType* expType=exp->get_type();
  string expTypeString=Backstroke::Utility::unparseTypeToString(expType);
  // because avpushptr returns a pointer to the original pointer, we need to cast to a pointer to the original type.
  string castOp="("+expTypeString+"*"+")";

  switch(_commandLineOptions->optionRTSSAccessMode()) {
  case 1:
    cerr<<"Error: Access mode 1 not supported in backend 2."<<endl;
    exit(1);
  case 2:
    return string("(")+generateRTSSAccess()+"avpushT("+Utility::unparseExprToString(exp)+"))";
  case 3:
    return string("(*"+castOp+generateRTSSAccess()+"avpushptr((void**)&(")+Utility::unparseExprToString(exp)+")))";
  default:
    ROSE_ASSERT(0);
  }
}

string Backstroke::TransformationSequence::generateAVPush(SgExpression* exp) {
  switch(_commandLineOptions->optionRTSSAccessMode()) {
  case 1:
    cerr<<"Error: Access mode 1 not supported in backend 2."<<endl;
    exit(1);
  case 2:
    return string("(")+generateRTSSAccess()+"avpushT("+Utility::unparseExprToString(exp)+"))";
  case 3:
    return string("(*"+generateRTSSAccess()+"avpush(&(")+Utility::unparseExprToString(exp)+")))";
  default:
    ROSE_ASSERT(0);
  }
}

// case enum to int. The returned int is implicitely cast to enum (in C++).
string Backstroke::TransformationSequence::generateEnumPush(SgExpression* exp) {
  return string("(*"+generateRTSSAccess()+"avpush(&((int)")+Utility::unparseExprToString(exp)+")))";
}

// this function generates code that: applies the address operator to
// the reference r and casts the result of the avpushptr function to a
// pointer to the base type of the reference r and dereferences it.
string Backstroke::TransformationSequence::generateBuiltInTypeRefPush(SgExpression* exp, SgType* refBaseType) {
  string refBaseTypeString=Utility::unparseTypeToString(refBaseType);
  string castExp="("+refBaseTypeString+"*)";
  return string("(*("+castExp+generateRTSSAccess()+"avpushptr((void**)&(")+Utility::unparseExprToString(exp)+"))))";
}

// determine if inside a template instantiation and return template instantiation node or 0.
// a node is inside a compiler generated template instantiation if it is not in a specialization (= 
SgNode* Backstroke::TransformationSequence::isInsideCompilerGeneratedTemplateInstantiation(SgNode* startNode) {
  SgNode* node=startNode;
  while(!Backstroke::Utility::isTemplateInstantiationNode(node)) {
    if(isSgProject(node)) {
      return 0;
    }
    node=node->get_parent();
  }
  // here: TemplateInstantion* node has been found

  // if the found template instantiation is not compiler generated
  // then startNode is not inside a compiler generated template instantiation (return 0)
  if(SgLocatedNode* locNode=isSgLocatedNode(node)) {
    if(!locNode->isCompilerGenerated()) {
      return 0;
    }
  }
  return node;
}

string Backstroke::TransformationSequence::computeSourceFragment(int startLine, int startCol, int endLine, int endCol, OriginalFile& originalFile) {
  string fragment="empty fragment";
  ROSE_ASSERT(startLine>=0);
  ROSE_ASSERT(startCol>=0);
  ROSE_ASSERT(endLine>=0);
  ROSE_ASSERT(endCol>=0);
  if(startLine==endLine) {
    int len=endCol-startCol+1;
    ROSE_ASSERT(!(((size_t)startLine)>originalFile.size()));
    if(((size_t)startLine)>originalFile.size()) {
      cerr<<"ERROR: transformation not possible: startLine beyond file ending: startLine:"<<startLine<<" lines in file: "<<originalFile.size()<<endl;
      exit(1);
    }
#if 1
    ROSE_ASSERT(!(((size_t)startCol+len)>originalFile[startLine].size()+1));
#else
    if( (((size_t)startCol+len)) > (originalFile[startLine].size()+1) ) {
      cerr<<"Error: !(((size_t)startCol+len)>originalFile[startLine].size()+1)\n"<<"startCol:"<<startCol<<" len: "<<len<<" startLine: "<<startLine<<" length: "<<originalFile[startLine].size()<<endl;
      exit(1);
    }
#endif

    // +1 is necessary to allow insertions past the line ending +1 but not more
    if(((size_t)startCol+len)>originalFile[startLine].size()+1) {
      //cerr<<"Number of lines in file: "<<originalFile.size()<<endl;
      cerr<<"ERROR: transformation not possible: out of bounds source code location in line "<<startLine<<endl;
      exit(1);
    }
    fragment=originalFile[startLine].substr(startCol,len);
  } else {
    cerr<<"WARNING: multi line fragment: not transformed at "<<startLine<<","<<startCol<<" => "<<endLine<<","<<endCol<<endl;
    fragment="/* ERROR MLF */";
  }
  return fragment;
}

Backstroke::Utility::LineCol 
Backstroke::TransformationSequence::findNextChar(int startLine,int startCol,char c, Backstroke::OriginalFile const& originalFile) {
  ROSE_ASSERT(startLine>=0);
  while((size_t)startLine<originalFile.size()) {
    int lineLen=originalFile[startLine].length();
    while(startCol<lineLen) {
      if(originalFile[startLine][startCol]==c) {
        return Backstroke::Utility::LineCol(startLine,startCol);
      }
      startCol++;
    }
    ROSE_ASSERT(startCol==lineLen);
    startLine++;
    startCol=0;
  }
  return Backstroke::Utility::LineCol();
}

std::string Backstroke::TransformationSequence::rtssFunctionCallCode() {
  switch(_commandLineOptions->optionRTSSAccessMode()) {
  case 1:
    ROSE_ASSERT(_rtssNamespaceName!="");
    return _rtssNamespaceName+"::";
  case 2:
  case 3:
    ROSE_ASSERT(_rtssVariableName!="" && _rtssAccessOperator!="");
    return generateRTSSAccess();
  default:
    cerr<<"Error: inconsistent backsroke rtss function call configuration in backend."<<endl;
    cerr<<"rtss variable : "<<_rtssVariableName<<endl;
    cerr<<"rtss access op: "<<_rtssAccessOperator<<endl;
    cerr<<"rtss namespace: "<<_rtssNamespaceName<<endl;
    exit(1);
  }
  return "gcc-dummy"; // unreachable code, but g++ 4.8.4 complains about control reaching end of function.
}

void Backstroke::TransformationSequence::applyToFile(OriginalFile& originalFile) {
  for(TransformationSequenceList::iterator i=transformationSequenceList.begin();i!=transformationSequenceList.end();++i) {
    TransformationOperation op=(*i).first;
    SgNode* ast0=(*i).second;
    ROSE_ASSERT(ast0);
    SgLocatedNode* ast=isSgLocatedNode(ast0);
    if(!ast) {
      cerr<<"Error: attempted to transform non-sg located node ("<<ast0->class_name()<<")"<<endl;
      exit(1);
    }
    // this adjusts (this->Exp) to Exp if this-> is compiler generated
    if(SgExpression* exp=isSgExpression(ast)) {
      ast=adjustCompilerGeneratedExpr(exp);
    }
    Sg_File_Info* fi=ast->get_file_info();
    ROSE_ASSERT(fi);
    string functionName;
    ROSE_ASSERT(ast);
    //transformSource(oldLineNrEnd, oldColNrEnd, lineNrStart, colNrStart, lineNrEnd, colNrEnd, functionName);
    if(SgLocatedNode* locNode=isSgLocatedNode(ast)) {
      int edgCorr=-1;
      int startLine=locNode->get_startOfConstruct()->get_raw_line()+edgCorr;
      int startCol=locNode->get_startOfConstruct()->get_raw_col()+edgCorr;
      int endLine=locNode->get_endOfConstruct()->get_raw_line()+edgCorr;
      int endCol=locNode->get_endOfConstruct()->get_raw_col()+edgCorr;
      if(isSgExpression(locNode) && locNode->isCompilerGenerated()) {
        if(_commandLineOptions->optionShowTransformationTrace()) {
          cout<<"GENERATED: "<<startLine<<":"<<startCol<<": skipping."<<endl;
        }
      } else {
        string functionName;
        _transformationStatistics.increment(op);
        bool applyTransformation=true;
        switch(op) {
        case TRANSOP_ASSIGNMENT:
          functionName="avpushT";
          if(SgExpression* exp=isSgExpression(locNode)) {
            if(_commandLineOptions->optionOptLocalVar() && isLocalVariable(exp)) {
              /* instrumentation is optimized to not being introduced */
              _transformationStatistics.decrement(op); // undo preceding increment
              if(_commandLineOptions->optionShowTransformationTrace()) {
                cout<<"TRACE: local variable access optimization:"
                    <<SgNodeHelper::sourceLineColumnToString(exp)
                    <<" (variable: "
                    <<exp->unparseToString()
                    <<")"
                    <<endl;
              }
              applyTransformation=false;
            } else {
              applyTransformation=true;
            }
          } else {
            applyTransformation=true;
          }
          break;
        case TRANSOP_MEM_ALLOC:
          functionName="registerAllocationT";
          break;
        case TRANSOP_MEM_ALLOC_ARRAY: {
          functionName="ERROR";
          
          SgNewExp* newExp=isSgNewExp(ast);
          ROSE_ASSERT(newExp);

          // qualification for the unparsing of the array type
          string s=Utility::unparseExprToString(newExp);

          // delete: (startLine,startCol) to (endLine,endCol);
          _editSequence.addToDelete(startLine,startCol,endLine,endCol);

          SgArrayType* arrayType=isSgArrayType(newExp->get_specified_type());
          ROSE_ASSERT(arrayType);
          string s2=Utility::unparseTypeToString(arrayType);
          if(_commandLineOptions->optionDebug()) cout<<"DEBUG: array allocation: array type: "<<s2<<endl;

          SgType* arrayElementType=arrayType->get_base_type();
          string arrayElementTypeString=Utility::unparseTypeToString(arrayElementType,newExp);
          if(_commandLineOptions->optionDebug()) cout<<"DEBUG: array allocation: element type: "<<arrayElementTypeString<<endl;

          SgExpression* expr=arrayType->get_index();
          expr=adjustCompilerGeneratedExpr(expr);

          // this compute line/col info allows to copy the original
          // array-size expression to the target file (currently not
          // used)
          stringstream  ssInfo;
          {
            int tstartLine=expr->get_startOfConstruct()->get_raw_line();
            int tstartCol=expr->get_startOfConstruct()->get_raw_col();
            int tendLine=expr->get_endOfConstruct()->get_raw_line();
            int tendCol=expr->get_endOfConstruct()->get_raw_col();
            ssInfo<<"/*"<<tstartLine<<":"<<tstartCol<<"=>"<<tendLine<<":"<<tendCol<<"*/";
          }
          string arraySizeString=Utility::unparseExprToString(expr);
          string toInsertString="allocateArrayT<"+arrayElementTypeString+">("+arraySizeString+")";
          _editSequence.addToInsert(startLine,startCol,"("+rtssFunctionCallCode()+toInsertString+")"+ssInfo.str());

          break;
        }
        case TRANSOP_MEM_DELETE: {
          functionName="registerDeallocationT";
          SgExpression* deleteNode=isSgExpression(locNode);
          ROSE_ASSERT(deleteNode);
          //SgdeleteNode=isSgExpression(exp->get_parent());
          ROSE_ASSERT(isSgDeleteExp(deleteNode));
          // delete: (startLineD,startColD) to (operand.StartLine,operand.startCol-1);
          SgExpression* operand=isSgExpression(SgNodeHelper::getFirstChild(deleteNode));
          ROSE_ASSERT(operand);
          operand=adjustCompilerGeneratedExpr(operand);
          int startLineOp=operand->get_startOfConstruct()->get_raw_line()+edgCorr;
          int startColOp=operand->get_startOfConstruct()->get_raw_col()+edgCorr;
          if(_commandLineOptions->optionDebug()) cout<<"DEBUG: delete operator operand: "<<startLineOp<<":"<<startColOp<<endl;
          _editSequence.addToDelete(startLine,startCol,startLineOp,startColOp-1);
          break;
        }
        case TRANSOP_MEM_DELETE_ARRAY: {
          functionName="registerArrayDeallocationT";
          SgExpression* deleteNode=isSgExpression(locNode);
          ROSE_ASSERT(deleteNode);
          //deleteNode=isSgExpression(exp->get_parent());
          ROSE_ASSERT(isSgDeleteExp(deleteNode));
          // delete: (startLineD,startColD) to (operand.StartLine,operand.startCol-1);
          SgExpression* operand=isSgExpression(SgNodeHelper::getFirstChild(deleteNode));
          ROSE_ASSERT(operand);
          operand=adjustCompilerGeneratedExpr(operand);
          int startLineOp=operand->get_startOfConstruct()->get_raw_line()+edgCorr;
          int startColOp=operand->get_startOfConstruct()->get_raw_col()+edgCorr;
          if(_commandLineOptions->optionDebug()) {
            cout<<"DEBUG: arr operand: "<<Utility::unparseExprToString(operand)<<" "<<startLineOp<<":"<<startColOp<<endl;
            cout<<"DEBUG: arr operand: deleteNode: "<<deleteNode->unparseToString()<<endl;
            cout<<"DEBUG: arr operand: operandNode: "<<operand->unparseToString()<<endl;
            cout<<"DEBUG: arr operand: operandNode: "<<AstTerm::astTermWithNullValuesToString(operand)<<endl;
          }
          _editSequence.addToDelete(startLine,startCol,startLineOp,startColOp-1);
          break;
        }
        case TRANSOP_CLASS_DEF: {
          SgClassDefinition* classDef=isSgClassDefinition(locNode);
          if(_commandLineOptions->optionDebug()) cout<<"DEBUG: class def begin: "<<classDef<<endl;
#if 1
          ClassInfo ci=determineClassInfo(classDef);
          ci.setRTSSAccess(this->rtssFunctionCallCode());

          if(ci.injectionLine()>=0 && ci.numberOfDataMembers()>0) {
            stringstream ss;
            //ss<<"/*BACKSTROKE GENERATED:"<<ci.injectionLine()<<","<<ci.injectionCol()<<"*/"; // infoline
            _editSequence.addToInsert(ci.injectionLine(),
                                      ci.injectionCol(), 
                                      ss.str()+ci.implAllDefaultOperators(classDef,
                                                                 _commandLineOptions,
                                                                 _dataMemberTypesInsideUnions
                                                                 )
                                      );
	    if(ci.generatedDefaultAssignOp())
	      _transformationStatistics.generatedDefaultAssignOpNum++;
          }
          // Generate names for all anonymous classes/structs/unions to allow constructors to be named
          if(ci.isAnonymous()) {
	    if(_commandLineOptions->optionUnion()==false && ci.isUnion()) {
	      // skip unions if command line option for ignoring unions is used
	    } else {
	      if(_commandLineOptions->optionDebug()) cout<<"DEBUG: found anonymous: "<<ci.className()<<":"<<startLine<<","<<startCol<<endl;
	      _editSequence.addToInsert(startLine,startCol+ci.keyWordLength()," "+ci.className()+" ");
	    }
          }
#endif
          if(ci.injectionLine()>=0 && ci.hasDefaultProtectedOrPrivateDestructor()) {
            if(_commandLineOptions->optionDebug()) cout<<"DEBUG: detected protected/private destructor. Generated friend declaration (OK).";
            _editSequence.addToInsert(ci.injectionLine(),ci.injectionCol(),"friend class Backstroke::RunTimeStateStore;\n");
          }

          if(ci.numBitFields()>0) {
            ClassInfo::BitFieldDeclarationList bfDeclList=ci.getBitFieldDeclarationList();	    
            for(ClassInfo::BitFieldDeclarationList::iterator i=bfDeclList.begin();i!=bfDeclList.end();++i) {
              //cout<<"WARNING: bitfield detected in class "<<ci.className()<<"::"<<(*i)->unparseToString()<<endl;
            }
          }
          break;
        }
        case TRANSOP_FUNCALL_OP_NEW:
          //cout<<"TRANSOP_FUNCTION_CALL: "<<startLine<<":"<<startCol<<" => "<<endLine<<":"<<endCol<<endl;
          functionName="registerOperatorNewT";
          break;
        case TRANSOP_FUNCALL_OP_DELETE: {
          if(_commandLineOptions->optionShowTransformationTrace()) {
            cout<<"TRANSOP_FUNCTION_CALL: "<<startLine<<":"<<startCol<<" => "<<endLine<<":"<<endCol<<endl;
          }
          functionName="operatorDeleteT";
          SgFunctionCallExp* funCallExp=isSgFunctionCallExp(locNode);
          // delete: (startLineD,startColD) to (operand.StartLine,operand.startCol-1);
          SgExprListExp* exprListExp=funCallExp->get_args();
          const SgExpressionPtrList& exprPtrList=exprListExp->get_expressions();
          if(!(exprPtrList.size()==1||exprPtrList.size()==2)) {
            cerr<<"Error: ::operator delete with less than 1 or more than 2 arguments."<<endl;
            exit(1);
          }
          SgExpression* operand=isSgExpression(*exprPtrList.begin());
          ROSE_ASSERT(operand);
          operand=adjustCompilerGeneratedExpr(operand);
          int startLineOp=operand->get_startOfConstruct()->get_raw_line()+edgCorr;
          int startColOp=operand->get_startOfConstruct()->get_raw_col()+edgCorr;
          //cout<<"DEBUG: operand: "<<startLineOp<<":"<<startColOp<<endl;
          _editSequence.addToDelete(startLine,startCol,startLineOp,startColOp-1);
          _editSequence.addToInsert(startLine,
                                    startCol,
                                    rtssFunctionCallCode()+functionName+"(");
          // the closing brace does not need to be added, because it already exists from the original (replaced) function call
          break;
        }
        case TRANSOP_MEMBER_FUNCTION_DECL: {
          // not considered
          break;
        }


        default:
          cerr<<"Error: unknown transformation operation."<<endl;
          exit(1);
        }
        if(applyTransformation 
           &&(
              op==TRANSOP_ASSIGNMENT
              ||op==TRANSOP_MEM_ALLOC
              ||op==TRANSOP_MEM_DELETE
              ||op==TRANSOP_MEM_DELETE_ARRAY
              ||op==TRANSOP_FUNCALL_OP_NEW
              )
           ) {
          if(_commandLineOptions->optionShowTransformationTrace()) {
            cout<<"BEGIN "<<endl;
          }
          if(_commandLineOptions->optionShowTransformationTrace()) {
            cout<<"transformation sequence operation "<<op
                <<" at: "<<startLine<<":"<<startCol
                <<" to "<<endLine<<":"<<endCol
                <<" EDIT: "<< (rtssFunctionCallCode()+functionName)<<endl;
          }                               
          string sourceFragment=computeSourceFragment(startLine,startCol,endLine,endCol, originalFile);
          if(sourceFragment=="/* ERROR */") {
            if(_commandLineOptions->optionShowTransformationTrace()) {
              cout<<"SKIPPING transformation."<<endl;
            }            
            continue;
          }
          if(_commandLineOptions->optionShowTransformationTrace()) {
            cout<<"transformation sequence operation "<<op
                <<" at: "<<startLine<<":"<<startCol
                <<" to "<<endLine<<":"<<endCol
                <<" EDIT: "<< (rtssFunctionCallCode()+functionName+"("+sourceFragment+")")<<endl;
          }
          _editSequence.addToInsert(startLine,
                                    startCol,
                                    "("+rtssFunctionCallCode()+functionName+"(");
          _editSequence.addToInsert(endLine,endCol+1,"))");
          if(_commandLineOptions->optionShowTransformationTrace()) {
            cout<<"END"<<endl;
          }
        }
      }
    } // exp
  }
}

bool Backstroke::TransformationSequence::isPointerType(SgType* type) {
  return isSgPointerType(type);
}

bool Backstroke::TransformationSequence::isReferenceType(SgType* type) {
  return isSgReferenceType(type);
}

bool Backstroke::TransformationSequence::isLocalVariable(SgExpression* exp) {
  if(SgVarRefExp* varRef=isSgVarRefExp(exp)) {
    SgSymbol* sym=SgNodeHelper::getSymbolOfVariable(varRef);
    if(sym) {
      SgScopeStatement* scope=sym->get_scope();
      if(isSgBasicBlock(scope)
         ||isSgCatchOptionStmt(scope)
         ||isSgDoWhileStmt(scope)
         ||isSgForStatement(scope)
         ||isSgFunctionDefinition(scope)
         ||isSgIfStmt(scope)
         ||isSgSwitchStatement(scope)
         ||isSgWhileStmt(scope)) {
        // ensure variable is not a reference variable
        SgType* type=varRef->get_type();
        if(!(SageInterface::isReferenceType(type)
             ||SageInterface::isPointerType(type)
             ||SageInterface::isVolatileType(type)
             )
           ) {
          return true;
        }
      } else {
        // not local
      }
    }
  }
  return false;
}

bool Backstroke::TransformationSequence::isArrayType(SgType* type) {
  return isSgArrayType(type);
}

bool Backstroke::TransformationSequence::isClassType(SgType* type) {
  return isSgClassType(type);
}

bool Backstroke::TransformationSequence::isEnumType(SgType* type) {
  return isSgEnumType(type);
}

SgType* Backstroke::TransformationSequence::getReferenceBaseType(SgType* type) {
  if(SgReferenceType* refType=isSgReferenceType(type)) {
    SgType* refBaseType=refType->get_base_type();
    return refBaseType;
  } else {
    return 0;
  }
}


void Backstroke::TransformationSequence::setRTSSAccessOperatorArrow() {
  _rtssAccessOperator="->"; 
}

void Backstroke::TransformationSequence::setRTSSAccessOperatorDot() {
  _rtssAccessOperator=".";
}

void Backstroke::TransformationSequence::setRTSSNamespaceName(std::string name) {
  _rtssNamespaceName=name;
}

Backstroke::TransformationStatistics::TransformationStatistics() {
  init();
}

void Backstroke::TransformationStatistics::init() {
  for(int i=0;i<TRANSOP_NUM;i++) {
    operatorsCount[i]=0;
  }
  generatedDefaultAssignOpNum=0;
}

void Backstroke::TransformationStatistics::increment(TransformationOperation transOp) {
  operatorsCount[transOp]+=1;
}

void Backstroke::TransformationStatistics::decrement(TransformationOperation transOp) {
  operatorsCount[transOp]-=1;
}

string Backstroke::TransformationStatistics::toString() {
  stringstream ss;
  std::string transformationOperationName[]={ "Assignment", "MemoryAlloc", "MemoryAllocArray", 
                                              "MemoryDelete", "MemoryDeleteArray", "Classdef","DeclBitField","FunCallDelete","FunCallNew","MemberFunctionDecl" };
  int total=0;
  cerr<<"PRINTING STATS"<<endl;
  for(int i=0;i<TRANSOP_NUM;i++) {
    ss<<transformationOperationName[i]<<": "<<operatorsCount[i]<<endl;
    total+=operatorsCount[i];
  }
  ss<<"DefaultAssignOpNum: "<<generatedDefaultAssignOpNum<<endl;
  ss<<"Total: "<<total<<endl;
  return ss.str();
}

string Backstroke::TransformationStatistics::toCsvString() {
  stringstream ss;
  int total=0;
  for(int i=0;i<TRANSOP_NUM;i++) {
    if(i!=0) {
      ss<<";";
    }
    ss<<operatorsCount[i];
    total+=operatorsCount[i];
  }
  ROSE_ASSERT(TRANSOP_NUM>0);
  ss<<";"<<total<<endl;
  return ss.str();
}


SgExpression* Backstroke::TransformationSequence::adjustCompilerGeneratedExpr(SgExpression* expr) {
  // match if this is the compiler-generated pattern 'this->originalExpression' and normalize
  if(expr->isCompilerGenerated()) {
    if(_commandLineOptions->optionDebug()) cout<<"DEBUG: adjustCompilerGeneratedExpr: compiler generated: "<<expr->unparseToString()<<endl;
    if(SgArrowExp* arrowExp=isSgArrowExp(expr)) {
      SgExpression* lhs=isSgExpression(SgNodeHelper::getLhs(arrowExp));
      SgExpression* rhs=isSgExpression(SgNodeHelper::getRhs(arrowExp));
      if(_commandLineOptions->optionDebug()) cout<<"DEBUG: adjustCompilerGeneratedExpr: isSgThisExp(lhs):"<<isSgThisExp(lhs)<<" compiler generated: LHS: "<<lhs->isCompilerGenerated()<<" RHS: "<<rhs->isCompilerGenerated()<<endl;
      if(isSgCastExp(lhs)&&lhs->isCompilerGenerated()) {
        lhs=isSgCastExp(lhs)->get_operand();
      }
      if(isSgThisExp(lhs)&&lhs->isCompilerGenerated()&&!rhs->isCompilerGenerated()) {
        // set expr to refer to the original expr
        if(_commandLineOptions->optionDebug()) cout<<"DEBUG: adjustCompilerGeneratedExpr: compiler generated: adjusted expr to: "<<rhs->unparseToString()<<endl;
        return rhs;
      }
    }
  }
  return expr;
}
