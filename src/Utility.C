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

#include "sage3basic.h"
#include <iostream>
#include <cstdlib>
#include "Utility.h"
#include "RoseAst.h"
#include "SgNodeHelper.h"

using namespace std;

using Backstroke::Utility::LineNr;
using Backstroke::Utility::ColNr;

std::pair<std::vector<SgExpression*>, SgType*> get_C_array_dimensions_aux(const SgArrayType& arr_type);

void Backstroke::Utility::printRoseInfo(SgProject* project) {
  project->display("PROJECT NODE");
  int fileNum=project->numberOfFiles();
  for(int i=0;i<fileNum;i++) {
    std::stringstream ss;
    SgFile* file=(*project)[i];
    ROSE_ASSERT(file);
    ss<<"FILE NODE Nr. "<<i;
    file->display(ss.str());
  }
}

std::set<SgNode*> Backstroke::Utility::subTreeNodes(SgNode* node) {
  std::set<SgNode*> res;
  RoseAst ast(node);
  for(RoseAst::iterator i=ast.begin();i!=ast.end();++i) {
    res.insert(*i);
  }
  return res;
}

size_t Backstroke::Utility::numberOfFiles(SgProject* project) {
  const SgFilePtrList& filePtrList=getFilePtrList(project);
  return filePtrList.size();
}

const SgFilePtrList& Backstroke::Utility::getFilePtrList(SgProject* project) {
  SgFileList* fileListPtr=project->get_fileList_ptr();
  ROSE_ASSERT(fileListPtr);
  const SgFilePtrList& filePtrList=fileListPtr->get_listOfFiles();
  return filePtrList;
}

SgFile* Backstroke::Utility::singleSourceFile(SgProject* project) {
  if(numberOfFiles(project)!=1)
    return 0;
  else {
    const SgFilePtrList& filePtrList=getFilePtrList(project);
    ROSE_ASSERT(filePtrList.size()==1);
    return isSgFile(*filePtrList.begin());
  }
}

std::string Backstroke::Utility::singleSourceFileName(SgProject* project) {
  SgFile* file=Backstroke::Utility::singleSourceFile(project);
  if(file) {
    return file->getFileName();
  } else {
    return "";
  }
}

SgClassDeclaration* Backstroke::Utility::isSpecialization(SgNode* node) {
  if(SgClassDeclaration* classDecl=isSgClassDeclaration(node)) {
    if(classDecl->isSpecialization()) {
      return classDecl;
    }
  }
  return 0;
}

bool Backstroke::Utility::isTemplateInstantiationNode(SgNode* node) {
  return isSgTemplateInstantiationDecl(node)
    || isSgTemplateInstantiationDefn(node)
    || isSgTemplateInstantiationFunctionDecl(node)
    || isSgTemplateInstantiationMemberFunctionDecl(node)
    || isSgTemplateInstantiationTypedefDeclaration(node)
    || isSgTemplateInstantiationDirectiveStatement(node)
    ;
}
 

// moved
void Backstroke::Utility::replaceString(std::string& str, const std::string& from, const std::string& to) {
#if 1
  SgNodeHelper::replaceString(str,from,to);
#else
  if(from.empty())
    return;
  size_t start_pos = 0;
  while((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
  }
#endif
}

// moved
SgNodeHelper::PragmaList
Backstroke::Utility::collectPragmaLines(string pragmaName,SgNode* root) {
#if 1
  return SgNodeHelper::collectPragmaLines(pragmaName,root);
#else
  PragmaList l;
  RoseAst ast(root);
  for(RoseAst::iterator i=ast.begin();i!=ast.end();++i) {
    std::list<SgPragmaDeclaration*> pragmaNodes;
    ROSE_ASSERT(pragmaNodes.size()==0);
    // SgPragmaDeclaration(SgPragma),..., SgStatement
    // collects consecutive pragma declarations
    while(i!=ast.end()&&(isSgPragmaDeclaration(*i)||isSgPragma(*i))) {
      if(SgPragmaDeclaration* pragmaDecl=isSgPragmaDeclaration(*i)) {
        pragmaNodes.push_back(pragmaDecl);
      }
      ++i;
    }
    if(i!=ast.end()) {
      for(std::list<SgPragmaDeclaration*>::iterator p=pragmaNodes.begin();p!=pragmaNodes.end();++p) {
        string str=Backstroke::Utility::getPragmaDeclarationString(*p);
        replaceString(str,"#pragma ","");
        if(Backstroke::Utility::isPrefix(pragmaName,str)) {
          SgPragmaDeclaration* lastPragmaDecl=pragmaNodes.back();
          // ensure we did not collect pragmas at the end of a block
          if(!(isLastChildOf(lastPragmaDecl,lastPragmaDecl->get_parent()))) {
            if(SgStatement* assocStmt=isSgStatement(*i)) {
              replaceString(str,pragmaName+" ","");
              //cout<<"PRAGMA REVERSE: "<<str<<" : "<<(assocStmt)->unparseToString()<<endl;
              l.push_back(make_pair(str,assocStmt));
            } else {
              std::cerr<<"Error: "<<SgNodeHelper::sourceLineColumnToString(*p)<<": reverse pragma not associated with a method or statement."<<endl
                       <<"Pragma         : "<<str<<endl
                       <<"Associated code: "<<assocStmt->unparseToString()<<endl;
              exit(1);
            }
          } else {
            std::cerr<<"Error: "<<SgNodeHelper::sourceLineColumnToString(*p)<<": reverse pragma at end of block. This is not allowed."<<endl;
            exit(1);
          }
        }
      }
    }
  }
  return l;
#endif
}

// moved
bool Backstroke::Utility::isPrefix( const std::string& prefix, const std::string& s )
{
#if 1
  return SgNodeHelper::isPrefix(prefix,s);
#else
  return std::equal(
                    prefix.begin(),
                    prefix.begin() + prefix.size(),
                    s.begin() 
                    );
#endif
}

// moved
bool Backstroke::Utility::isLastChildOf(SgNode* elem, SgNode* parent) {
#if 1
  return isLastChildOf(elem,parent);
#else
  std::vector<SgNode*> children=parent->get_traversalSuccessorContainer();
  return elem==children.back();
#endif
}

// moved
std::string Backstroke::Utility::getPragmaDeclarationString(SgPragmaDeclaration* pragmaDecl) {
#if 1
  return SgNodeHelper::getPragmaDeclarationString(pragmaDecl);
#else
  SgPragma* pragma=pragmaDecl->get_pragma();
  ROSE_ASSERT(pragma);
  return pragma->get_pragma();
#endif
}

void Backstroke::Utility::markNodeToBeUnparsed(SgLocatedNode* node) {
  // special case: implicit casts are never to be generated (in ROSE: marked as 'compiler generated').
  if(SgCastExp* castExp=isSgCastExp(node)) {
    if(castExp->isCompilerGenerated()) {
      return;
    }
  }
  node->setTransformation();
  node->setOutputInCodeGeneration();
}

bool Backstroke::Utility::isMarkedNodeToBeUnparsed(SgNode* node) {
  if(SgLocatedNode* lnode=isSgLocatedNode(node)) {
    return lnode->isTransformation() /*||lnode->isOutputInCodeGeneration()*/;
  } else {
    return false;
  }
}

/*! 
  * \author Markus Schordan
  * \date 2012.
 */
list<SgFunctionDefinition*> Backstroke::Utility::listOfFunctionDefinitions(SgNode* node) {
  list<SgFunctionDefinition*> funDefList;
  RoseAst ast(node);
  for(RoseAst::iterator i=ast.begin();i!=ast.end();++i) {
    if(SgFunctionDefinition* funDef=isSgFunctionDefinition(*i)) {
      funDefList.push_back(funDef);
    }
  }
  return funDefList;
}

void Backstroke::Utility::writeStringToFile(std::string data, std::string fileName) {
  ofstream myfile;
  myfile.open(fileName.c_str());
  myfile << data;
  myfile.close();
}

size_t Backstroke::Utility::findNonWhiteSpacePos(std::string data) {
  size_t i=0;
  while(i<data.size() && std::isspace(data[i++]))
    ;
  return i;
}

// sub is the substring to search for, str is the string to search in.
std::vector<size_t> Backstroke::Utility::findSubstringOccurences(std::string sub, std::string str) {
  vector<size_t> positions; // holds all the positions that sub occurs within str
  size_t pos = str.find(sub, 0);
  while(pos != string::npos) {
    positions.push_back(pos);
    pos = str.find(sub,pos+1);
  }
  return positions;
}

Backstroke::Utility::LineCol::LineCol():line(0),col(0),valid(false) {
  
}
Backstroke::Utility::LineCol::LineCol(LineNr line, ColNr col):line(line),col(col),valid(true) {
}

std::string Backstroke::Utility::LineCol::toString() {
  std::stringstream ss;
  ss<<line<<":"<<col;
  return ss.str();
}

Backstroke::Utility::LineColName::LineColName(LineNr line, ColNr col, std::string name, bool isNamespaceName, bool isGlobalNamespaceName)
  :_lineCol(LineCol(line,col)),
   _name(name),
   _isNamespaceName(isNamespaceName),
   _isGlobalNamespaceName(isGlobalNamespaceName) {
  ROSE_ASSERT(!(_isGlobalNamespaceName && !_isNamespaceName));
    }

bool Backstroke::Utility::LineColName::isNamespaceName() { return _isNamespaceName; }

bool Backstroke::Utility::LineColName::isGlobalNamespaceName() { return _isGlobalNamespaceName; }

Backstroke::Utility::LineCol Backstroke::Utility::LineColName::getLineCol() { return _lineCol; }

std::string Backstroke::Utility::LineColName::getName() { return _name; }

std::string Backstroke::Utility::LineColName::toString() {
  return "["
    +getLineCol().toString()
    +":"+getName()+":"
    +(isNamespaceName()?"true":"false")
    +":"
    +(isGlobalNamespaceName()?"true":"false")
    +"]";
}
std::string Backstroke::Utility::LineColName::getFullName() {
  std::string s;
  if(isGlobalNamespaceName()) {
    s+="::";
  }
  s+=getName();
  if(isNamespaceName()) {
    s+="::";
  }
  return s;
}

size_t Backstroke::Utility::LineColName::getFullNameLength() {
  return getFullName().length();
}

size_t Backstroke::Utility::LineColName::getNameLength() {
  return getName().length();
}

std::string Backstroke::Utility::SplicedName::toString() {
  std::string s;
  for(std::vector<LineColName>::iterator i=this->begin();
      i!=this->end();
      ++i) {
      if(i!=this->begin())
	s+=",";
      s+=(*i).toString();
  }
  return s;
}
std::string Backstroke::Utility::SplicedName::getFullName() {
  std::string s;
  for(std::vector<LineColName>::iterator i=this->begin();
      i!=this->end();
      ++i) {
    s+=(*i).getFullName();
  }
  return s;
}
size_t Backstroke::Utility::SplicedName::getFullNameLength() {
  return getFullName().length();
}

// determines namespace names and name starting at position col
Backstroke::Utility::SplicedName 
Backstroke::Utility::spliceName(std::string s, Backstroke::Utility::LineNr line, Backstroke::Utility::ColNr colStart) {
  SplicedName res;
  ColNr colEnd=colStart;
  bool foundNamespaceName=false;
  do {
    // a namespace name can be begin with "::";
    if(foundNamespaceName==false && static_cast<size_t>(colEnd)+2<=s.length()) {
      if(s[colEnd]==':' && s[colEnd+1]==':') {
	foundNamespaceName=true;
	colEnd+=2;
	colStart=colEnd;
      }
    } else {
      foundNamespaceName=false;
    }
    while(std::isalpha(s[colEnd])||isdigit(s[colEnd])||s[colEnd]=='_') {
      colEnd++;
    }
    if(foundNamespaceName) {
      ROSE_ASSERT(colEnd-colStart>=0);
      res.push_back(LineColName(line,colStart,s.substr(colStart,colEnd-colStart),true,true));
      if(colEnd+2<=s.length()) {
	if(s[colEnd]==':' && s[colEnd+1]==':') {
	  colEnd+=2;
	  colStart=colEnd;
	}
      }
    } else {
      // immeditely following the name we must find a "::" (then it is a
      // namespace name)
      if(colEnd+2<=s.length() && s[colEnd]==':' && s[colEnd+1]==':') {
	foundNamespaceName=true;
        ROSE_ASSERT(colEnd-colStart>=0);
	res.push_back(LineColName(line,colStart,s.substr(colStart,colEnd-colStart),true,false));
	colEnd+=2;
	colStart=colEnd;
      } else {
	// check for a non-spacename name
	if(colEnd>colStart) {
          ROSE_ASSERT(colEnd-colStart>=0);
	  res.push_back(LineColName(line,colStart,s.substr(colStart,colEnd-colStart),false,false));	
	  // must be at end
	  break;
	}
      }
    }
  } while(foundNamespaceName);
  return res;
}


std::vector<SgExpression*>
Backstroke::Utility::getArrayDimensionExpressions(const SgArrayType& arr_type) {
  namespace SB = SageBuilder;
  namespace SI = SageInterface;
  
  const SgArrayType* arrtype = &arr_type;
  std::vector<SgExpression*> arrayDimensionExpressions;
  SgType* undertype = NULL;
  
  // \todo when get_index() does not return a nullptr anymore
  //       the condition can be removed
  if (arrtype->get_index() == NULL) {
    arrayDimensionExpressions.push_back(SB::buildNullExpression());
    undertype = arrtype->get_base_type();
    arrtype = isSgArrayType(undertype);
  }
  
  while (arrtype) {
    SgExpression* indexexpr = arrtype->get_index();
    ROSE_ASSERT(indexexpr);
    arrayDimensionExpressions.push_back(SI::deepCopy(indexexpr));
    undertype = arrtype->get_base_type();
    while(SgTypedefType* typeDefType=isSgTypedefType(undertype)) {
      undertype=typeDefType->get_base_type();
    }
    arrtype = isSgArrayType(undertype);
  }
  
  ROSE_ASSERT((!arrayDimensionExpressions.empty()) && undertype);
  return arrayDimensionExpressions;
}

void Backstroke::Utility::deleteArrayDimensionExpressions(ArrayDimensionExpressions arrayDimensionExpressions) {
  for(ArrayDimensionExpressions::iterator i=arrayDimensionExpressions.begin();i!=arrayDimensionExpressions.end();++i) {
    delete *i;
  }
}

Unparser_Opt Backstroke::Utility::defaultRoseOptions() {
  // also see: unparser.C : inheritedAttributeInfoPointer->set_forceQualifiedNames();
  bool _auto                         = false;
  bool linefile                      = false;
  bool useOverloadedOperators        = false;
  bool num                           = false;
  
  // It is an error to have this always turned off (e.g. pointer = this; will not unparse correctly)
  bool _this                         = true;
  
  bool caststring                    = false;
  bool _debug                        = false;
  bool _class                        = false;
  bool _forced_transformation_format = false;
  bool _unparse_includes             = false;

  Unparser_Opt roseOptions( _auto,
                            linefile,
                            useOverloadedOperators,
                            num,
                            _this,
                            caststring,
                            _debug,
                            _class,
                            _forced_transformation_format,
                            _unparse_includes );
  return roseOptions;
}

// BE1: the type of an array allocation is the only type that needs to
// be unparsed (but this is currently not supported in ROSE).  this
// method therefore calls directly into the ROSE backend after setting
// up the required context (which is usually done by the ROSE backend
// when unparsing the expression that contains the type. 
// BE2: same as BE1, but some more additional cases where types are
// unparsed (most in access-mode 3).
std::string Backstroke::Utility::unparseTypeToString(SgType* type, SgNode* referenceNodeForNameQualification) {
#if 0
  SgUnparse_Info info;
  info.set_SkipEnumDefinition();
  info.set_SkipClassDefinition();
  string s=type->unparseToString(&info);
  // workaround 1
  Utility::replaceString(s,"class ::","");
  return s;
#else
  Unparser_Opt roseOptions=defaultRoseOptions();
  ostringstream outputString;
  string fileNameOfStatementsToUnparse="defaultFileNameInGlobalUnparseToString";
  Unparser roseUnparser ( &outputString, fileNameOfStatementsToUnparse, roseOptions );
  SgUnparse_Info* inheritedAttributeInfoPointer=new SgUnparse_Info();
  initializeSgUnparseInfo(inheritedAttributeInfoPointer);
  SgSourceFile* sourceFile = TransformationSupport::getSourceFile(type);
  inheritedAttributeInfoPointer->set_current_source_file(sourceFile);
  SgUnparse_Info & inheritedAttributeInfo = *inheritedAttributeInfoPointer;

  //inheritedAttributeInfo.unset_PrintName();
  //inheritedAttributeInfo.unset_isTypeFirstPart();
  //inheritedAttributeInfo.unset_isTypeSecondPart();
  //inheritedAttributeInfo.set_SkipClassSpecifier();

  // unparseCxx_expressions.C: L5538 (new operator)
  inheritedAttributeInfo.set_reference_node_for_qualification(referenceNodeForNameQualification);

  roseUnparser.u_type->unparseType ( const_cast<SgType*>(type), inheritedAttributeInfo );
  //cout<<"DEBUG: unparseToString: "<<outputString.str()<<endl;
  string s=outputString.str();
  // workaround 1
  Utility::replaceString(s,"class ::","");
  return s;
#endif
}

void Backstroke::Utility::initializeSgUnparseInfo(SgUnparse_Info* inheritedAttributeInfoPointer) {
  inheritedAttributeInfoPointer->set_SkipEnumDefinition();
  inheritedAttributeInfoPointer->set_SkipClassDefinition();
  inheritedAttributeInfoPointer->set_SkipComments();
  inheritedAttributeInfoPointer->set_SkipWhitespaces();
  inheritedAttributeInfoPointer->set_SkipCPPDirectives();
  inheritedAttributeInfoPointer->set_outputCompilerGeneratedStatements();
  inheritedAttributeInfoPointer->unset_forceQualifiedNames();
  inheritedAttributeInfoPointer->unset_CheckAccess();
}

std::string Backstroke::Utility::unparseExprToString(SgExpression* expr) {
#if 0
  SgUnparse_Info info;
  info.set_SkipEnumDefinition();
  info.set_SkipClassDefinition();
  return expr->unparseToString(&info);
#else
  Unparser_Opt roseOptions=defaultRoseOptions();
  ostringstream outputString;
  string fileNameOfStatementsToUnparse="defaultFileNameInGlobalUnparseToString";
  Unparser roseUnparser ( &outputString, fileNameOfStatementsToUnparse, roseOptions );
  SgUnparse_Info* inheritedAttributeInfoPointer=new SgUnparse_Info();
  initializeSgUnparseInfo(inheritedAttributeInfoPointer);
  SgSourceFile* sourceFile = TransformationSupport::getSourceFile(expr);
  inheritedAttributeInfoPointer->set_current_source_file(sourceFile);
  SgUnparse_Info & inheritedAttributeInfo = *inheritedAttributeInfoPointer;
  roseUnparser.u_exprStmt->unparseExpression ( const_cast<SgExpression*>(expr), inheritedAttributeInfo );

  //cout<<"DEBUG: unparseToString: "<<outputString.str()<<endl;
  return outputString.str();

#endif
}

size_t Backstroke::Utility::getArrayElementCount(SgArrayType* arrayType) {
  size_t arraySize;
  SgExpression * indexExp =  arrayType->get_index();
  SgUnsignedLongVal * valExp = isSgUnsignedLongVal(indexExp);
  SgIntVal * valExpInt = isSgIntVal(indexExp);
  // guard execution of SageInterface function that may fail
  if(valExp||valExpInt)
    arraySize=SageInterface::getArrayElementCount(arrayType);
  else
    arraySize=0;
  return arraySize;
}

void Backstroke::Utility::showClassDefinition(SgClassDefinition* classDef) {
  const SgDeclarationStatementPtrList& memberList=classDef->get_members();
  SgName className=classDef->get_qualified_name();
  bool hasAssignOp=false;
  for(SgDeclarationStatementPtrList::const_iterator i=memberList.begin();i!=memberList.end();++i) {
    if(SgMemberFunctionDeclaration* memFunDecl=isSgMemberFunctionDeclaration(*i)) {
      // determine overloaded operators
      SgName qualMemFunName=memFunDecl->get_qualified_name();
      SgName memFunName=memFunDecl->get_name();
      const SgFunctionModifier& funMod=memFunDecl->get_functionModifier();
      funMod.isDefault();
      funMod.isInline();
      funMod.isExplicit();
      funMod.isVirtual();
      const SgSpecialFunctionModifier& specialFunMod=memFunDecl->get_specialFunctionModifier();
      // http://rosecompiler.org/ROSE_HTML_Reference/classSgSpecialFunctionModifier.html
      specialFunMod.isDefault();
      specialFunMod.isConstructor();
      specialFunMod.isConversion();
      specialFunMod.isOperator();
      // copy constructor: A::A(const A&);
      // copy assign op: void A::operator=(const A &other ) { var1=other.var1;...; }
      bool isAssignOp=false;
      if(memFunName=="operator=") {
        isAssignOp=true;
        hasAssignOp=true;
      }
#if 1
      cout<<"INFO: "<<qualMemFunName<<":"
          <<memFunName<<":"
        //<<funMod.isDefault()<<"|"
        //  <<specialFunMod.isDefault()
          <<specialFunMod.isConstructor()
          <<specialFunMod.isConversion()
          <<specialFunMod.isOperator()
          <<((isAssignOp)?"ASSIGN-OPEATOR!":"")
          <<endl;
#endif
    }
  }
  cout<<"ASSIGN-OP ";
  if(!hasAssignOp) {
    cout<<"MISSING: ";
  } else {
    cout<<"PRESENT: ";
  }
  cout<<className<<":"<<classDef<<endl;
  typedef std::vector< std::pair< SgNode*, std::string > > DataMemberPointers;
  DataMemberPointers  dataMemPtrs=classDef->returnDataMemberPointers();
  stringstream copyAssignOpImpl;
  for(DataMemberPointers::iterator i=dataMemPtrs.begin();i!=dataMemPtrs.end();++i) {
    // create list of assignments and initializers (check for variables vs methods)
    SgNode* node=(*i).first;
    string name=(*i).second;
    if(SgVariableDeclaration* varDecl=isSgVariableDeclaration(node)) {
      const SgInitializedNamePtrList& initNameList=varDecl->get_variables();
      for(SgInitializedNamePtrList::const_iterator i=initNameList.begin();i!=initNameList.end();++i) {
        string var=(*i)->get_name();
        copyAssignOpImpl<<"this->"<<var<<"=other."<<var<<endl;
      }
    }
  }
  cout<<"COPYASSIGNOP-IMPL:"<<copyAssignOpImpl.str();
}

void Backstroke::Utility::showGlobalDeclarations(SgNode* node) {
  /* SgTypedefDeclaration
     SgClassDeclaration
   */
  RoseAst ast(node);
  for(RoseAst::iterator i=ast.begin();i!=ast.end();++i) {
    if(SgDeclarationStatement* declStmt=isSgDeclarationStatement(*i)) {
      SgScopeStatement* scope=declStmt->get_scope();
      if(isSgGlobal(scope) && !declStmt->isCompilerGenerated()) {
        cout<<"Declaration in global scope: "<<declStmt->class_name()<<":"<<declStmt->unparseToString().substr(0,40)<<endl;
        if(SgFunctionDeclaration* funDecl=isSgFunctionDeclaration(declStmt)) {
          SgName fName=funDecl->get_name();
          const SgSpecialFunctionModifier& funMod=funDecl->get_specialFunctionModifier();
          if(funMod.isOperator()) {
            int edgCorr=-1;
            int startLine=funDecl->get_startOfConstruct()->get_raw_line()+edgCorr;
            int startCol=funDecl->get_startOfConstruct()->get_raw_col()+edgCorr;
            int endLine=funDecl->get_endOfConstruct()->get_raw_line()+edgCorr;
            int endCol=funDecl->get_endOfConstruct()->get_raw_col()+edgCorr;
            SgFunctionDefinition* funDef=funDecl->get_definition();
            cout<<"Found global operator declaration."<<fName<<" "<<startLine<<":"<<startCol<<" to "<<endLine<<":"<<endCol<<"::"<<funDef<<endl;
          }
        }
      }
    } else {
      if(isSgNamespaceDeclarationStatement(*i)||isSgFunctionDefinition(*i)) {
        i.skipChildrenOnForward();
      }
    }
  }
}

bool Backstroke::Utility::isInsideNamespace(SgDeclarationStatement* declStmt) {
  SgNode* node=declStmt;
  while (!isSgGlobal(node)&&!isSgProject(node)) {
    node=node->get_parent();
    if(isSgNamespaceDeclarationStatement(node)) {
      return true;
    }
  };
  return false;
}
bool Backstroke::Utility::checkRoseVersionNumber(const std::string &need) {
    std::vector<std::string> needParts = Rose::StringUtility::split('.', need);
    std::vector<std::string> haveParts = Rose::StringUtility::split('.', ROSE_PACKAGE_VERSION);
    for (size_t i=0; i < needParts.size() && i < haveParts.size(); ++i) {
      int needPartInt=std::stoi(needParts[i]);
      int havePartInt=std::stoi(haveParts[i]);
      if (needPartInt != havePartInt)
	return needPartInt < havePartInt;
    }

    // E.g., need = "1.2" and have = "1.2.x", or vice versa
    return true;
} 
