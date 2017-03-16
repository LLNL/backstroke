#include "sage3basic.h"
#include "PragmaMap.h"

using namespace std;

#include "pragmaparser_decls.h"
// bison parser
extern int pragmaparserparse();
// bison parser generated AST
extern Backstroke::Annotation* pragmaAst; 

Backstroke::Annotation::Annotation() {}
Backstroke::Annotation::~Annotation() {}

Backstroke::PragmaMap::PragmaMap() {
}

void Backstroke::PragmaMap::addPragmaList(SgNodeHelper::PragmaList& pragmaList) {
  for(SgNodeHelper::PragmaList::iterator i=pragmaList.begin();
      i!=pragmaList.end();
      ++i) {
    bool res=addPragma((*i).first,(*i).second);
    if(res==false) {
      cerr<<"Error: wrong syntax in pragma: "<<(*i).first<<endl;
      exit(1);
    }
  }
}

bool Backstroke::PragmaMap::addPragma(string str, SgNode* node) {
  // call bison parser
  pragmaAst=0;
  InitializeParser(str);
  pragmaparserparse();
  FinishParser();

  // exclude directive applies to the entire subtree
  if(AnnotationExclude* annotationExclude=dynamic_cast<Backstroke::AnnotationExclude*>(pragmaAst)) {
    //std::cout<<"INFO: AnnotationExclude detected: "<<str<<endl;
    std::set<SgNode*> subTreeNodeSet=Backstroke::Utility::subTreeNodes(node);
    for(std::set<SgNode*>::iterator i=subTreeNodeSet.begin();
        i!=subTreeNodeSet.end();
        ++i) {
      insert(make_pair(*i,annotationExclude));
      //cout<<"DEBUG: ADDING excl:"<<(*i)->unparseToString()<<endl;
    }
    return true;
  } else {
    return false;
  }
}

bool Backstroke::PragmaMap::isExcludedNode(SgNode* node) {
  Backstroke::Annotation* annot=(*this)[node];
  return dynamic_cast<Backstroke::AnnotationExclude*>(annot);
}
bool Backstroke::PragmaMap::isFunctionCallToBeReversed(SgNode* node) {
  Backstroke::Annotation* annot=(*this)[node];
  return dynamic_cast<Backstroke::AnnotationFunctionCallToBeReversed*>(annot);
}
