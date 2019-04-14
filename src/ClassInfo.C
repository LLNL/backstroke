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
#include "ClassInfo.h"
#include "Utility.h"

#include "RoseAst.h"

using namespace std;

/*
  SgDeclarationStatement
  virtual SgScopeStatement * 	get_scope (void) const 
  std::string 	get_linkage ()
  const SgDeclarationModifier & 	get_declarationModifier () const 
  SgDeclarationModifier:
  const SgTypeModifier & 	get_typeModifier () const
  const SgAccessModifier & 	get_accessModifier () const
  const SgStorageModifier & 	get_storageModifier () const

  SgDeclarationModifier
*/

Backstroke::ClassInfo::ClassInfo():
  _defaultConstructor(false),
  _defaultCopyConstructor(false),
  _anyCopyAssignOperator(false),
  _defaultCopyAssignOperator(false),
  _defaultMoveAssignOperator(false),
  _defaultProtectedOrPrivateDestructor(false),
  _classDecl(0),
  _classDef(0),
  _hasConstDataMember(false),
  _hasCLinkage(false)
{
}

Backstroke::ClassInfo::~ClassInfo() {
}
  
std::string Backstroke::ClassInfo::name() {
  return _className;
}

vector<SgInitializedNamePtrList*> Backstroke::ClassInfo::getConstructorInitializerLists() {
  vector<SgInitializedNamePtrList*> cList;
  DataMemberPointers dataMemPtrs=_classDef->returnDataMemberPointers();
  for(DataMemberPointers::iterator i=dataMemPtrs.begin();i!=dataMemPtrs.end();++i) {
    // create list of assignments and initializers (check for variables vs methods)
    SgNode* node=(*i).first;
    string name=(*i).second;
    if(SgMemberFunctionDeclaration* memFunDecl=isSgMemberFunctionDeclaration(node)) {
      const SgSpecialFunctionModifier& specialFunMod=memFunDecl->get_specialFunctionModifier();
      // http://rosecompiler.org/ROSE_HTML_Reference/classSgSpecialFunctionModifier.html
      if(specialFunMod.isConstructor()) {
        const SgInitializedNamePtrList& ctors=memFunDecl->get_ctors();
        for(SgInitializedNamePtrList::const_iterator i=ctors.begin();i!=ctors.end();++i) {
          //TODO
#if 0
          LineColInfo mflcInfo=TransformationSequence::lineColInfo(memFunDecl);
          //cout<<"DEBUG: constructor:"<<className()<<" : LOC:"<<mflcInfo.toString()<<": ";
          string varName=(*i)->get_name();
          SgInitializer* initializer=(*i)->get_initializer();
          SgInitializedName::preinitialization_enum preInitEnum=(*i)->get_preinitialization();
          /*
            e_virtual_base_class = 1,
            e_nonvirtual_base_class = 2,
            e_data_member = 3,
            e_delegation_constructor = 4, 
          */
          LineColInfo lcInfo=TransformationSequence::lineColInfo(initializer);
          cout<<" init:"<<preInitEnum<<":"<<varName<<"="<<initializer->unparseToString()<<"LOC:"<<lcInfo.toString()<<endl;
          SgNodeHelper::replaceAstWithString(initializer,"INIT!");
#endif
        }
        //cout<<endl;
        //cout<<"DEBUG: MemFunDeclString:"<<memFunDecl->unparseToString()<<endl;
      }
    }
  }
  return cList;
}

bool Backstroke::ClassInfo::isPThreadRelevantClass() {
  std::string s=className();
  return s.find("pthread")!=string::npos
    ||s.find("mutex")!=string::npos
    ;
}

//#define WORKAROUND1
bool Backstroke::ClassInfo::isRefTypeOfClass(SgType* argType,SgClassDefinition* classDef) {
  if(SgReferenceType* refType=isSgReferenceType(argType)) {
    SgType* argBaseType=refType->get_base_type();
    if(SgClassType* argClassType=isSgClassType(argBaseType)) {
      SgClassDeclaration* classDecl=classDef->get_declaration();
      return argClassType==classDecl->get_type();
    }
  }
  return false;
}

bool Backstroke::ClassInfo::isConstRefTypeOfClass(SgType* argType,SgClassDefinition* classDef) {
  if(SgReferenceType* refType=isSgReferenceType(argType)) {
    SgType* argBaseType=refType->get_base_type();
    if(SgModifierType* modifierType=isSgModifierType(argBaseType)) {
      // TODO: check for 'const'
      SgType* modifierBaseType=modifierType->get_base_type();
      if(SgClassType* argClassType=isSgClassType(modifierBaseType)) {
        SgClassDeclaration* classDecl=classDef->get_declaration();
        return argClassType==classDecl->get_type();
      }
    }
  }
  return false;
}

bool Backstroke::ClassInfo::hasDefaultConstructor() {
  return _defaultConstructor;
}
void Backstroke::ClassInfo::setDefaultConstructor() {
  _defaultConstructor=true;
}
bool Backstroke::ClassInfo::hasDefaultCopyConstructor() {
  return _defaultCopyConstructor;
}
void Backstroke::ClassInfo::setDefaultCopyConstructor() {
  _defaultCopyConstructor=true;
}
bool Backstroke::ClassInfo::hasAnyCopyAssignOperator() {
  return _anyCopyAssignOperator;
}
void Backstroke::ClassInfo::setAnyCopyAssignOperator() {
  _anyCopyAssignOperator=true;
}
bool Backstroke::ClassInfo::hasDefaultCopyAssignOperator() {
  return _defaultCopyAssignOperator;
}
void Backstroke::ClassInfo::setDefaultCopyAssignOperator() {
  _defaultCopyAssignOperator=true;
}
bool Backstroke::ClassInfo::hasDefaultMoveAssignOperator() {
  return _defaultMoveAssignOperator;
}
void Backstroke::ClassInfo::setDefaultMoveAssignOperator() {
  _defaultMoveAssignOperator=true;
}
void Backstroke::ClassInfo::setDefaultProtectedOrPrivateDestructor() {
  _defaultProtectedOrPrivateDestructor=true;
}
bool Backstroke::ClassInfo::hasDefaultProtectedOrPrivateDestructor() {
  return _defaultProtectedOrPrivateDestructor;
}


std::list<SgVariableDeclaration*> Backstroke::ClassInfo::getDataMembers() {
  std::list<SgVariableDeclaration*> varDeclList;
  DataMemberPointers dataMemPtrs=_classDef->returnDataMemberPointers();
  // returnDataMemberPointers includes all declarations (methods need to be filtered)
  for(DataMemberPointers::iterator i=dataMemPtrs.begin();i!=dataMemPtrs.end();++i) {
    SgNode* node=(*i).first;
    if(SgVariableDeclaration* varDecl=isSgVariableDeclaration(node)) {
      varDeclList.push_back(varDecl);
    }
  }
  return varDeclList;
}

size_t Backstroke::ClassInfo::numberOfDataMembers() {
  return getDataMembers().size();
}

bool Backstroke::ClassInfo::hasConstDataMember() {
  return _hasConstDataMember;
}

bool Backstroke::ClassInfo::hasCLinkage() {
  return _hasCLinkage;
}

void Backstroke::ClassInfo::initializeDataMemberProperties() {
  DataMemberPointers  dataMemPtrs=_classDef->returnDataMemberPointers();
  _hasConstDataMember=false;
  for(DataMemberPointers::iterator i=dataMemPtrs.begin();i!=dataMemPtrs.end();++i) {
    SgNode* node=(*i).first;
    if(SgVariableDeclaration* varDecl=isSgVariableDeclaration(node)) {
#if 1
      string linkage=varDecl->get_linkage();
      if(linkage=="C") {
        _hasCLinkage=true;
      }
#endif
      const SgInitializedNamePtrList& initNameList=varDecl->get_variables();
      for(SgInitializedNamePtrList::const_iterator i=initNameList.begin();i!=initNameList.end();++i) {
        SgType* varType=(*i)->get_type();
        ROSE_ASSERT(varType);
        _hasConstDataMember=_hasConstDataMember||SageInterface::isConstType(varType);
      }
    }
  }
  determineBitFields();
}

SgAccessModifier Backstroke::ClassInfo::getFirstDataMemberAccessModifier() {
  SgVariableDeclaration* node=firstDataMember();
  if(node) {
    return node->get_declarationModifier().get_accessModifier();
  } else {
    // otherwise return default access modifier (by default 'e_unknown')
    SgAccessModifier am;
    return am;
  }
}

SgVariableDeclaration* Backstroke::ClassInfo::firstDataMember() {
  ROSE_ASSERT(_classDef);
  std::list<SgVariableDeclaration*> dataMemPtrs=getDataMembers();
  std::list<SgVariableDeclaration*> ::iterator i=dataMemPtrs.begin();
  if(i!=dataMemPtrs.end()) {
    return *i;
  }
  return 0;
}

int Backstroke::ClassInfo::injectionLine() {
  int edgCorr=-1;
  SgVariableDeclaration* locNode=firstDataMember();
  if(locNode) {
    return locNode->get_startOfConstruct()->get_raw_line()+edgCorr;
  }
  return -1;
}

int Backstroke::ClassInfo::injectionCol() {
  int edgCorr=-1;
  // TODO: check access level of next data member (or beginning of class definition)
  SgVariableDeclaration* locNode=firstDataMember();
  if(locNode)
    return locNode->get_startOfConstruct()->get_raw_col()+edgCorr;
  return -1;
}

bool Backstroke::ClassInfo::generatedDefaultAssignOp() {
  return _generatedDefaultAssignOp;
}

std::string Backstroke::ClassInfo::implAllDefaultOperators(SgClassDefinition* classDef, CommandLineOptions* clo, std::set<SgType*>& dataMemberTypesInsideUnions) {
  std::string impl;
  _generatedDefaultAssignOp=false;
  if(!hasConstDataMember()&&!hasCLinkage()) {
    if(!hasDefaultConstructor()&&clo->optionGenerateDefaultConstructor()) {
      impl+=implConstructor();
    }
    if(!hasDefaultCopyConstructor()&&clo->optionGenerateDefaultCopyConstructor()) {
      impl+=implCopyConstructor();
    }
    // TODO: refine check for DefaultCopyAssignOperator and CopyAssignOperator
    // this check may be overly conservative: need to check whether a copy assign op exists that conflicts with a default copy assign op
    if(!hasAnyCopyAssignOperator()&&!hasDefaultCopyAssignOperator()&&clo->optionGenerateDefaultAssignOp()) {
      // by default unions are addressed (optionUnion==true). The false-case is only for experimental purposes for c++98 code.
      if(clo->optionUnion()==true) {
        impl+=implCopyAssignOperator();
	_generatedDefaultAssignOp=true;
      } else {
	// allow to ignore unions
      }
    }
  }
  return impl;
}
void Backstroke::ClassInfo::setRTSSAccess(std::string s) {
  _rtssAccess=s;
}

std::string Backstroke::ClassInfo::getRTSSAccess() {
  return _rtssAccess;
}

std::size_t Backstroke::ClassInfo::numBitFields() {
  return _bitFieldVarDecls.size();
}

Backstroke::ClassInfo::BitFieldDeclarationList Backstroke::ClassInfo::getBitFieldDeclarationList() {
  return _bitFieldVarDecls;
}

void Backstroke::ClassInfo::determineBitFields() {
  DataMemberPointers dataMemPtrs=_classDef->returnDataMemberPointers();
  for(DataMemberPointers::iterator i=dataMemPtrs.begin();i!=dataMemPtrs.end();++i) {
    SgNode* node=(*i).first;
    string name=(*i).second;
    if(SgVariableDeclaration* varDecl=isSgVariableDeclaration(node)) {
      SgExpression* bitfieldValExp=varDecl->get_bitfield();
      if(bitfieldValExp!=0) {
	_bitFieldVarDecls.push_back(varDecl);
      }
    }
  }  
}

std::string Backstroke::ClassInfo::reversibleCopyCode() {
  bool copyConstructor=false;
  stringstream impl;
  DataMemberPointers  dataMemPtrs=_classDef->returnDataMemberPointers();
  for(DataMemberPointers::iterator i=dataMemPtrs.begin();i!=dataMemPtrs.end();++i) {
    // create list of assignments and initializers (check for variables vs methods)
    SgNode* node=(*i).first;
    string name=(*i).second;
    if(SgVariableDeclaration* varDecl=isSgVariableDeclaration(node)) {
      SgExpression* bitfieldValExp=varDecl->get_bitfield();
      const SgInitializedNamePtrList& initNameList=varDecl->get_variables();
      const SgDeclarationModifier& declModifier=varDecl->get_declarationModifier();
      const SgStorageModifier& storageModifier=declModifier.get_storageModifier();
      bool isStatic=storageModifier.isStatic();
      for(SgInitializedNamePtrList::const_iterator i=initNameList.begin();i!=initNameList.end();++i) {
        SgType* originalVarType=(*i)->get_type();
        ROSE_ASSERT(originalVarType);
        SgType* strippedVarType=originalVarType->stripTypedefsAndModifiers();
        if(SgArrayType* arrayType=isSgArrayType(strippedVarType)) {
          ROSE_ASSERT(arrayType);
          Utility::ArrayDimensionExpressions arrayDimensionExpressions=Backstroke::Utility::getArrayDimensionExpressions(arrayType);
          //cout<<"INFO: array varType:"<<Utility::unparseTypeToString(varType)<<", size:"<<arraySize<<endl;
          string arrayName=(*i)->get_name();
          if(copyConstructor) {
            size_t arraySize=Utility::getArrayElementCount(arrayType);
            for(unsigned int i=0;i<arraySize;i++) {
              // there is no way in C++98 for initializing an array in a constructor initializer list
              // C++11 allows A::A():arrayName{1, 2, 3},...
              cerr<<"Error: member variables of type array not supported in copy constructors."<<endl;
              exit(1);
            }
          } else {
            /* ROSE: build new vector if first element is of type SgNullExpression (should not exist)*/
            if(arrayDimensionExpressions.size()>0 && isSgNullExpression(*arrayDimensionExpressions.begin())) {
              // build new vector
              Utility::ArrayDimensionExpressions arrayDimensionExpressionsTmp;
              for(Utility::ArrayDimensionExpressions::iterator dimIter=arrayDimensionExpressions.begin();dimIter!=arrayDimensionExpressions.end();++dimIter) {
                if(!isSgNullExpression(*dimIter)) {
                  arrayDimensionExpressionsTmp.push_back(*dimIter);
                }
              }
              ROSE_ASSERT(arrayDimensionExpressions.size()==arrayDimensionExpressionsTmp.size()+1);
              arrayDimensionExpressions=arrayDimensionExpressionsTmp;
            }

            // dimension counter
            int d=1;
            string loopHeads;
            string arrayElementAccess=arrayName;
            string loopBody;
            string loopTails;
            for(std::vector<SgExpression*>::iterator dimIter=arrayDimensionExpressions.begin();dimIter!=arrayDimensionExpressions.end();++dimIter) {
              ROSE_ASSERT(!isSgNullExpression(*dimIter));
              stringstream ss;
              ss<<d;
              string iterVar="i"+ss.str();
              string iterVarType="int";
              loopHeads+="for("+iterVarType+" "+iterVar+"=0;"+iterVar+"!="+(*dimIter)->unparseToString()+";"+iterVar+"++) {\n";
              arrayElementAccess+="["+iterVar+"]";
              loopTails+="}\n";
              d++;
            }
            loopBody=getRTSSAccess()+"avpushT(this->"+arrayElementAccess+")=other."+arrayElementAccess+";\n";
            impl<<loopHeads<<loopBody<<loopTails;
            Utility::deleteArrayDimensionExpressions(arrayDimensionExpressions);
          }
        } else {
          if(!isStatic) {
            string var=(*i)->get_name();
            // TODO: investigate why variable names can be ""
            if(var!="") {
              if(copyConstructor) {
                if(i!=initNameList.begin()) {
                  impl<<","<<endl;
                }
                if(bitfieldValExp!=0) {
                  impl<<"/* bitfield: not copied */";
                  cout<<"WARNING: bitfield not supported yet. Ignored in assignment operator."<<endl;
                } else {
                  impl<<getRTSSAccess()<<"(avpushT(this->"<<var<<"),other."<<var<<")";
                }
                if(i+1==initNameList.end()) {
                  impl<<endl;
                }
              } else {
                if(bitfieldValExp!=0) {
                  impl<<"/* bitfield: not copy assigned */"<<endl;
                } else {
                  impl<<getRTSSAccess()<<"avpushT(this->"<<var<<")=other."<<var<<";"<<endl;
                }
              }
            }
          }
        }
      }
    }
  }
  return impl.str();
}

std::string Backstroke::ClassInfo::implConstructor() {
  stringstream impl;
  impl<<"public: ";
  impl<<_className<<"() {\n";
  impl<<"}\n";
  return impl.str();
}

std::string Backstroke::ClassInfo::implCopyAssignOperator() {
  stringstream impl;
  impl<<"public: ";
  impl<<_className<<"& operator=(const "<<_className<<"& other) {"<<endl;
  impl<<reversibleCopyCode();
  impl<<"return *this;"<<endl;
  impl<<"}"<<endl;
  return impl.str();
}

std::string Backstroke::ClassInfo::implCopyConstructor() {
  stringstream impl;
  impl<<"public: ";
  impl<<_className<<"(const "<<_className<<"& other) {\n";
  impl<<reversibleCopyCode();
  impl<<"}\n";
  return impl.str();
}

bool Backstroke::ClassInfo::isAnonymous() {
  return Utility::isPrefix("__anonymous_0x",_className);
}

bool Backstroke::ClassInfo::isUnionDeclaration(SgNode* node) {
  if(SgClassDeclaration* classDecl=isSgClassDeclaration(node)) {
    SgClassDeclaration::class_types classType=classDecl->get_class_type();
    return classType==SgClassDeclaration::e_union;
  }
  return false;
}

bool Backstroke::ClassInfo::isUnion() {
  SgClassDeclaration::class_types classType=_classDecl->get_class_type();
  return classType==SgClassDeclaration::e_union;
}

bool Backstroke::ClassInfo::isStruct() {
  SgClassDeclaration::class_types classType=_classDecl->get_class_type();
  return classType==SgClassDeclaration::e_struct;
}

bool Backstroke::ClassInfo::isClass() {
  SgClassDeclaration::class_types classType=_classDecl->get_class_type();
  return classType==SgClassDeclaration::e_class;
}

std::set<SgType*> Backstroke::ClassInfo::dataMemberTypesInsideUnions(SgNode* node) {
  std::set<SgType*> types;
  RoseAst ast(node);
  for(RoseAst::iterator i=ast.begin();i!=ast.end();++i) {
    if(SgClassDefinition* classDef=isSgClassDefinition(*i)) {
	  ClassInfo ci;
      ci.initialize(classDef);
      if(ci.isUnion()) {
        std::list<SgVariableDeclaration*> varDeclList=ci.getDataMembers();
        for(std::list<SgVariableDeclaration*>::iterator j=varDeclList.begin();
            j!=varDeclList.end();
            ++j) {
          SgInitializedNamePtrList& initNames=(*j)->get_variables ();
          // a declaration of a data member variable must have exactly one entry
          ROSE_ASSERT(initNames.size()==1);
          SgInitializedName* initNamePtr=*(initNames.begin());
          SgType* dataMemberType=initNamePtr->get_type();
          types.insert(dataMemberType);
        }
      }
    }
  }
  return types;
}

bool Backstroke::ClassInfo::isInsideUnion(SgClassDeclaration* classDecl) {
  SgNode* d=classDecl;;
  while(!(isSgProject(d) || d==0)) {
    if(ClassInfo::isUnionDeclaration(d)) {
      //cout<<"DEBUG: inside union: "<<d->unparseToString()<<endl;
      return true;
    } else {
      d=d->get_parent();
    }
  }
  return false;
}

bool Backstroke::ClassInfo::isInsideUnion() {
  return isInsideUnion(_classDecl);
}

std::string Backstroke::ClassInfo::className() {
  return _className;
}

int Backstroke::ClassInfo::keyWordLength() {
  if(isClass()) {return 5;}
  else if(isStruct()) {return 6;}
  else if(isUnion()) {return 5;}
  return 0;
}

void Backstroke::ClassInfo::initialize(SgClassDefinition* classDef) {
  _classDef=classDef;
  _classDecl=_classDef->get_declaration();
  _className=_classDecl->get_name(); // TODO2: see src/backend/unparser.C : unparseFile->generateNameQualificationSupport()
  string linkage=_classDecl->get_linkage();
  if(linkage=="C") {
    _hasCLinkage=true;
  }
  const SgDeclarationStatementPtrList& memberList=classDef->get_members();
  SgName className=classDef->get_qualified_name();
  for(SgDeclarationStatementPtrList::const_iterator i=memberList.begin();i!=memberList.end();++i) {
    if(SgMemberFunctionDeclaration* memFunDecl=isSgMemberFunctionDeclaration(*i)) {
      // determine overloaded operators
      SgName qualMemFunName=memFunDecl->get_qualified_name();
      SgName memFunName=memFunDecl->get_name();
      const SgFunctionModifier& funMod=memFunDecl->get_functionModifier();
      funMod.isInline();
      funMod.isExplicit();
      funMod.isVirtual();
      const SgSpecialFunctionModifier& specialFunMod=memFunDecl->get_specialFunctionModifier();
      // http://rosecompiler.org/ROSE_HTML_Reference/classSgSpecialFunctionModifier.html
      //specialFunMod.isConstructor();
      //specialFunMod.isConversion();
      //specialFunMod.isOperator();

      // check for default copy constructor: A::A(const A&);
      if(specialFunMod.isConstructor()) {
        const SgInitializedNamePtrList& funArgs=memFunDecl->get_args();
        if(funArgs.size()==0) {
          this->setDefaultConstructor();
        } else if(funArgs.size()==1) {
          SgInitializedName* initName=*funArgs.begin();
          SgType* argType=initName->get_type();
          // TODO: constness must be removed to become equal
          if(ClassInfo::isConstRefTypeOfClass(argType,classDef)) {
            this->setDefaultCopyConstructor();
          } else {
            // not default copy constructor
          }
        }
      }
      
      // copy assign op: A& A::operator=(const A &other ) { var1=other.var1;...; }
      // TODO: check return type
      if(specialFunMod.isOperator() && memFunName=="operator=") {
        const SgInitializedNamePtrList& funArgs=memFunDecl->get_args();
        ROSE_ASSERT(funArgs.size()==1);
        SgInitializedName* initName=*funArgs.begin();
        SgType* argType=initName->get_type();
        SgType* returnType=memFunDecl->get_orig_return_type();
        if(ClassInfo::isRefTypeOfClass(returnType,classDef) && ClassInfo::isConstRefTypeOfClass(argType,classDef)) {
          this->setDefaultCopyAssignOperator();
        }
        this->setAnyCopyAssignOperator();
      }
      if(specialFunMod.isDestructor()) {
        const SgInitializedNamePtrList& funArgs=memFunDecl->get_args();
        if(funArgs.size()==0) {
          SgAccessModifier accessMod=memFunDecl->get_declarationModifier().get_accessModifier();
          if(accessMod.isProtected()||accessMod.isPrivate()) {
            this->setDefaultProtectedOrPrivateDestructor();
          }
        }
      }
    }
  }
  initializeDataMemberProperties();
}
