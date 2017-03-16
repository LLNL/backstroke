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
#include "EditSequence.h"

#include "CodeGenerator.h"
#include "SgNodeHelper.h"
#include "RoseAst.h"

using namespace std;

bool Backstroke::EditSequenceElement::operator==(const EditSequenceElement other) const {
  return line==other.line && col==other.col;
}

bool Backstroke::EditSequenceElement::operator<(const EditSequenceElement other) const {
  if(line!=other.line)
    return line<other.line;
  return col<other.col;
}

void
Backstroke::EditSequence::addToInsert(int line, int col, std::string stringToInsert) {
  _editSequenceMap.insert(std::make_pair(EditSequenceElement(line,col),stringToInsert));
}

void
Backstroke::EditSequence::addToDelete(int startLine, int startCol, int endLine, int endCol, OriginalFile const& originalFile) {
  ROSE_ASSERT((size_t)startCol<=originalFile[startLine].length()-1);
  if(startLine<endLine) {
    int startLine2=startLine;
    int startCol2=startCol;
    while(startLine2<endLine) {
      int endCol2=originalFile[startLine2].length()-1;
      for(int i=startCol2;i<=endCol2;i++) {
        _toDeleteSet.insert(EditSequenceElement(startLine2,i));
      }
      startLine2++;
      startCol2=0;
    }
    ROSE_ASSERT(startLine2==endLine);
    addToDelete(startLine2,startCol2,endLine,endCol);
  } else {
    addToDelete(startLine,startCol,endLine,endCol);
  }
}

void
Backstroke::EditSequence::addToDelete(int startLine, int startCol, int endLine, int endCol) {
  // we could query the original file and compute beyond one line (ignoring newline)
  if(startLine!=endLine) {
    cerr<<"Error: startLine!=endLine:"<<startLine<<":"<<startCol<<" => "<<endLine<<endCol<<endl;
    exit(1);
  }
  ROSE_ASSERT(startLine==endLine);
  for(int i=startCol;i<=endCol;i++) {
    _toDeleteSet.insert(EditSequenceElement(startLine,i));
  }
}

bool
Backstroke::EditSequence::toDelete(int line, int col) {
  return _toDeleteSet.find(EditSequenceElement(line,col))!=_toDeleteSet.end();
}

string
Backstroke::EditSequence::toString() {
  stringstream ss;
  for(std::map<EditSequenceElement,string>::iterator i=_editSequenceMap.begin();
      i!=_editSequenceMap.end();
      ++i) {
    ss<<(*i).first.line<<":"<<(*i).first.col<<":"<<(*i).second<<endl;
  }
  return ss.str();
}

int
Backstroke::EditSequence::entryExists(int line, int col) {
  return _editSequenceMap.count(EditSequenceElement(line,col));
}

std::list<std::string>
Backstroke::EditSequence::getEntry(int line, int col) {
  list<string> strList;
  if(entryExists(line,col)) {
    std::pair<EditSequenceMapType::iterator, EditSequenceMapType::iterator> multiMapRange;
    multiMapRange = _editSequenceMap.equal_range(EditSequenceElement(line,col));
    for (EditSequenceMapType::iterator it=multiMapRange.first; it!=multiMapRange.second; ++it) {
      strList.push_back(it->second);
    }
    return strList;
  } else {
    return strList;
  }
}
