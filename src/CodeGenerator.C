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
#include <iostream>
#include <fstream>

#include "CodeGenerator.h"
#include "TransformationSequence.h"

#include "SgNodeHelper.h"
#include "RoseAst.h"

using namespace std;

void Backstroke::CodeGenerator::init() {
  _outputFileNamePrefix="backstroke_";
}

Backstroke::CodeGenerator::CodeGenerator():_commandLineOptions(0) {
  init();
}

Backstroke::CodeGenerator::CodeGenerator(CommandLineOptions* clo):_commandLineOptions(clo) {
  init();
}

void Backstroke::CodeGenerator::setCommandLineOptions(CommandLineOptions* clo) {
  _commandLineOptions=clo;
}

std::string Backstroke::CodeGenerator::originalFileContent() {
  std::string originalFileContent;
  for(size_t i=0;i<_originalFileContent.size();++i) {
    originalFileContent+=_originalFileContent[i]+"\n";
  }
  cout<<"DEBUG: orginal file content: "<<originalFileContent.size()<<" bytes."<<endl;
  return originalFileContent;
}

void Backstroke::CodeGenerator::setOutputFileNamePrefix(std::string outputFileNamePrefix) {
  _outputFileNamePrefix=outputFileNamePrefix;
}

std::string Backstroke::CodeGenerator::getOutputFileNamePrefix() {
  return _outputFileNamePrefix;
}
void Backstroke::CodeGenerator::setOutputFileName(std::string fileName) {
  _outputFileName=fileName;
}
void Backstroke::CodeGenerator::setFileNameWithPath(std::string fileName) {
  _fileNameWithPath=fileName;
  const string& str="/";
  size_t pathLastPos=_fileNameWithPath.find_last_of(str);
  if(pathLastPos<_fileNameWithPath.size()) {
    _filePath=_fileNameWithPath.substr(0,pathLastPos+1);
    _fileName=_fileNameWithPath.substr(pathLastPos+1,_fileNameWithPath.size()-pathLastPos);
  } else {
    _filePath="";
    _fileName=fileName;
  }
  _outputFileName=getFileName();
  // determine working dir (cstd library)
  char path[2048+1];
  string targetPath=std::string(getcwd(path,2048)); // creates a copy of path
  _outputFileNameWithPath=targetPath+"/"+_outputFileName;
}

std::string Backstroke::CodeGenerator::getFileName() {
  return _fileName;
}

std::string Backstroke::CodeGenerator::getFilePath() {
  return _filePath;
}

std::string Backstroke::CodeGenerator::getFileNameWithPath() {
  return _fileNameWithPath;
}

std::string Backstroke::CodeGenerator::getOutputFileName() {
  return _outputFileNamePrefix+_outputFileName;
}

std::string Backstroke::CodeGenerator::getOutputFileNameWithPath() {
  return _filePath+_outputFileNamePrefix+_outputFileName;
}

void Backstroke::CodeGenerator::loadFile() {
  ifstream myfile (_fileNameWithPath.c_str());
  if (myfile.is_open()) {
    if(_commandLineOptions->optionStatusMessages()) {
      cout<<"STATUS: loading file "<<getFileName()<<endl;
    }
    string line;
    //_originalFileContent.push_back("empty line");
    while ( getline (myfile,line) ) {
      //_originalFileContent.push_back(" "+line);
      _originalFileContent.push_back(line);
    }
    myfile.close();
  } else {
    cerr << "Error: Unable to open file "<<_fileName<<endl;
    exit(1);
  }
}

void Backstroke::CodeGenerator::saveEditedFileAs() {
  std::string finalFileContent=_editedFileContent;
  if(_commandLineOptions->optionStatusMessages()) {
    cout<<"STATUS: saving edited file as "<<getOutputFileNameWithPath()<<" ("<<finalFileContent.size()<<" bytes)"<<endl;
  }
  Backstroke::Utility::writeStringToFile(finalFileContent, getOutputFileNameWithPath().c_str());
}

//void Backstroke::CodeGenerator::forwardCodeTransformation(SgFunctionDefinition* funDef) {
void Backstroke::CodeGenerator::forwardCodeTransformation(SgNode* node) {
  if(_commandLineOptions) {
    _transformationSequence.setCommandLineOptions(_commandLineOptions);
    if(_commandLineOptions->optionAccessOperatorArrow()) {
      _transformationSequence.setRTSSAccessOperatorArrow();
    } else {
      _transformationSequence.setRTSSAccessOperatorDot();
    }
  }
  _transformationSequence.setRTSSVariableName("rts");
  _transformationSequence.setRTSSNamespaceName("xpdes");
  _transformationSequence.setPragmaMap(&_pragmaMap);
  //_transformationSequence.showGlobalDeclarations(funDef);
  _transformationSequence.create(node);
}

void Backstroke::CodeGenerator::markTemplateInstantiationToBeUnparsed(SgNode* node) {
  ROSE_ASSERT(Backstroke::Utility::isTemplateInstantiationNode(node));
  RoseAst subAst(node);
  for(RoseAst::iterator j=subAst.begin();j!=subAst.end();++j) {
    if(SgLocatedNode* locatedNode=isSgLocatedNode(*j)) {
      Backstroke::Utility::markNodeToBeUnparsed(locatedNode);
    }
  }
}

bool Backstroke::CodeGenerator::isUnmarkedTemplateNodeToBeUnparsed(SgNode* node) {
  return isSgTemplateInstantiationDecl(node)
    || isSgTemplateInstantiationDefn(node)
    || isSgTemplateInstantiationTypedefDeclaration(node)
    || isSgTemplateInstantiationDirectiveStatement(node);
}

int Backstroke::CodeGenerator::markAllTemplateInstantiationsToBeUnparsed(SgProject* root) {
  SgFile* fileNode=Backstroke::Utility::singleSourceFile(root);
  ROSE_ASSERT(fileNode);
  SgSourceFile* sourceFile=isSgSourceFile(fileNode);
  if(!sourceFile) {
    cerr<<"Error: input file is not a source file."<<endl;
    exit(1);
  }
  SgGlobal* global=sourceFile->get_globalScope();
  ROSE_ASSERT(global);
  RoseAst ast(global);
  int n=0;
#if 1
  for(RoseAst::iterator i=ast.begin();i!=ast.end();++i) {
    if(Backstroke::Utility::isTemplateInstantiationNode(*i)) {
      // if minimize option is not set mark in any case
      markTemplateInstantiationToBeUnparsed(*i);
      //i.skipChildrenOnForward();
      n++;
    }
  }
#else
      // global ROSE function
      // void markTransformationsForOutput(SgLocatedNode*);
      markTransformationsForOutput(node);
#endif
  return n;
}

void Backstroke::CodeGenerator::checkTemplateInstantiations(SgProject* root) {
  SgFile* fileNode=Backstroke::Utility::singleSourceFile(root);
  ROSE_ASSERT(fileNode);
  SgSourceFile* sourceFile=isSgSourceFile(fileNode);
  if(!sourceFile) {
    cerr<<"Error: input file is not a source file."<<endl;
    exit(1);
  }
  SgGlobal* global=sourceFile->get_globalScope();
  // Does this also provide local declarations inside functions?
  RoseAst ast(global);
  list<SgTemplateClassDeclaration*> templClassDecls;
  list<SgTemplateMemberFunctionDeclaration*> templMemberFunDecls;
  list<SgTemplateInstantiationDecl*> templInstDecls;
  list<SgTemplateInstantiationDefn*> templInstDefs;
  list<SgTemplateInstantiationFunctionDecl*> templInstFunDecls;
  list<SgTemplateInstantiationMemberFunctionDecl*> templInstMemberFunDecls;
  list<SgTemplateInstantiationTypedefDeclaration*> templInstTypedefDecls;
  list<SgTemplateInstantiationDirectiveStatement*> templInstDirectiveStmts;
  //cout<<"TCC:"<<SgNodeHelper::sourceLineColumnToString(t)<<endl;
  int templConstructCount=0;
  for(RoseAst::iterator i=ast.begin();i!=ast.end();++i) {
    bool isTemplConstruct=true;
    if(false) {
    } else if(SgTemplateClassDeclaration* t=isSgTemplateClassDeclaration(*i)) {
      templClassDecls.push_back(t);
    } else if(SgTemplateMemberFunctionDeclaration* t=isSgTemplateMemberFunctionDeclaration(*i)) {
      templMemberFunDecls.push_back(t);
    } else if(SgTemplateInstantiationDecl* t=isSgTemplateInstantiationDecl(*i)) {
      templInstDecls.push_back(t);
      //SgTemplateClassDeclaration* templClassDecl=t->get_templateDeclaration();
    } else if(SgTemplateInstantiationDefn* t=isSgTemplateInstantiationDefn(*i)) {
      templInstDefs.push_back(t);
    } else if(SgTemplateInstantiationFunctionDecl* t=isSgTemplateInstantiationFunctionDecl(*i)) {
      templInstFunDecls.push_back(t);
    } else if(SgTemplateInstantiationMemberFunctionDecl* t=isSgTemplateInstantiationMemberFunctionDecl(*i)) {
      templInstMemberFunDecls.push_back(t);
      //SgTemplateMemberFunctionDeclaration* templMemFuncDecl=t->get_templateDeclaration();
    } else if(SgTemplateInstantiationTypedefDeclaration* t=isSgTemplateInstantiationTypedefDeclaration(*i)) {
      templInstTypedefDecls.push_back(t);
    } else if(SgTemplateInstantiationDirectiveStatement* t=isSgTemplateInstantiationDirectiveStatement(*i)) {
      templInstDirectiveStmts.push_back(t);
    } else {
      isTemplConstruct=false;
    }
    if(isTemplConstruct) {
      templConstructCount++;
    }
  }
  bool templateErrorExit=false;
  if(_commandLineOptions->optionTemplateCheck() && (templConstructCount>0)
     &&(!_commandLineOptions->optionExperimental())) {
    templateErrorExit=true;
  }
  if(_commandLineOptions->optionStatusMessages()||templateErrorExit) {
    cout<<"INFO: Template Class Declarations                        : "<<templClassDecls.size()<<endl;
    cout<<"INFO: Template Member Function Declarations              : "<<templMemberFunDecls.size()<<endl;
    cout<<"INFO: Template Instantiation Declarations                : "<<templInstDecls.size()<<endl;
    cout<<"INFO: Template Instantiation Definitions                 : "<<templInstDefs.size()<<endl;
    cout<<"INFO: Template Instantiation Function Declarations       : "<<templInstFunDecls.size()<<endl;
    cout<<"INFO: Template Instantiation Member Function Declarations: "<<templInstMemberFunDecls.size()<<endl;
    cout<<"INFO: Template Instantiation Typedef Declarations        : "<<templInstTypedefDecls.size()<<endl;
    cout<<"INFO: Template Instantiation Directive Statements        : "<<templInstDirectiveStmts.size()<<endl;
    cout<<"INFO: Total number of template constructs                : "<<templConstructCount<<endl;
  }
  if(templateErrorExit) {
    cerr<<"Error: templates are not supported yet."<<endl;
    exit(1);
  }
  return;
}

void Backstroke::CodeGenerator::generateCode(SgProject* root) {
  //initNameQualification(root);
  SgNodeHelper::PragmaList pragmaList;
  // deprecated keyword "reverse"
  pragmaList=Backstroke::Utility::collectPragmaLines("reverse",root);
  _pragmaMap.addPragmaList(pragmaList);
  // general form
  pragmaList=Backstroke::Utility::collectPragmaLines("reversible",root);
  _pragmaMap.addPragmaList(pragmaList);
  //cout<<"INFO: pragma map size: "<<_pragmaMap.size()<<endl;

  ROSE_ASSERT(_commandLineOptions);
  if(_commandLineOptions->optionTransform()) {
    SgFile* fileRoot=Backstroke::Utility::singleSourceFile(root);
    forwardCodeTransformation(fileRoot);
    
    if(_commandLineOptions->optionTemplateCheck()) {
      checkTemplateInstantiations(root);
    }
    if(_commandLineOptions->optionExperimental()) {
      // use experimental mode for unparsing template instantiationsa
      //Unparse_MOD_SAGE::experimentalMode=true;
      //Unparse_MOD_SAGE::experimentalModeVerbose=1;
      int n=markAllTemplateInstantiationsToBeUnparsed(root);
      if(_commandLineOptions->optionStatusMessages()) {
        cout<<"STATUS: detected "<<n<<" template instantiations ";
        cout<<endl;
      }
    }
  } else {
    if(_commandLineOptions->optionStatusMessages()) {
      cout<<"STATUS: Transformations are turned off. Generating code without reversible-code transformations."<<endl;
    }
  }
  switch(int backend=_commandLineOptions->optionBackend()) {
  case 1: {
    _transformationSequence.applyToFile(_originalFileContent);
    if(_commandLineOptions->optionShowTransformationTrace()) {
      cout<<"------------ EDIT SEQUENCE -----------"<<endl;
      cout<<_transformationSequence.editSequenceToString()<<endl;
      cout<<"--------------------------------------"<<endl;
    }
    applyEditTransformationSequence();
    saveEditedFileAs();
    break;
  }
#ifdef BACKEND2
  case 2: {
    _transformationSequence.apply();
    SourceCodeFormat* sourceCodeFormat = new SourceCodeFormat();
    root->unparse(sourceCodeFormat,0);
    break;
  }
#endif
  default:
    cerr<<"Internal error: selected backend "<<backend<<" does not exist."<<endl;
    ROSE_ASSERT(false);
  }
  if(_commandLineOptions->optionStats()) {
    cout<<"Transformation Statistics:"<<endl;
    cout<<_transformationSequence.getTransformationStatistics().toString();
  }
  if(_commandLineOptions->optionCsvStatsFileName()!="") {
    Backstroke::Utility::writeStringToFile(_transformationSequence.getTransformationStatistics().toCsvString(),
                                          _commandLineOptions->optionCsvStatsFileName());
  }
}

void Backstroke::CodeGenerator::applyEditTransformationSequence() {
  if(_commandLineOptions->optionStatusMessages()) {
    cout<<"STATUS: applying edit transformation sequence."<<endl;
  }
  EditSequence& editSequence=_transformationSequence.editSequence();
  for(size_t lineCount=0;lineCount<_originalFileContent.size();lineCount++) {
    // iterator over all columns + 1: one additional column is handled
    // to allow for inserts after the last column in the same line
    for(size_t colCount=0;colCount<_originalFileContent[lineCount].size()+1;colCount++) {
      if(editSequence.entryExists(lineCount,colCount)) {
        // insert all strings that are registered to be inserted at this location
        list<string> strList=editSequence.getEntry(lineCount,colCount);
        for(list<string>::iterator i=strList.begin();i!=strList.end();++i) {
          _editedFileContent+=*i;
        }
      }
      // add the original code right after it
      if(colCount!=(_originalFileContent[lineCount].size()+1)-1) {
        // add it only if it is not supposed to be deleted
        if(!editSequence.toDelete(lineCount,colCount)) {
          _editedFileContent+=_originalFileContent[lineCount].substr(colCount,1);
        }
      } else {
        // special case: last column + 1 in line
        // nothing to add from original file 
      }
    }
    _editedFileContent+="\n";
  }
}

// function in ROSE unparser
extern void generateNameQualificationSupport( SgNode* node, std::set<SgNode*> & referencedNameSet );
// required for unparseToString function
void Backstroke::CodeGenerator::initNameQualification(SgProject* project) {
  SgFile* file=Backstroke::Utility::singleSourceFile(project);
  std::set<SgNode*> referencedNameSet;
  generateNameQualificationSupport(file,referencedNameSet);
  propagateHiddenListData(file);
}
