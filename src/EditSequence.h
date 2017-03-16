#ifndef EDIT_SEQUENCE_H
#define EDIT_SEQUENCE_H

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

#include <set>
#include <map>
#include <list>

namespace Backstroke {
  typedef std::vector<std::string> OriginalFile;

  struct EditSequenceElement {
  EditSequenceElement(int line, int col):line(line),col(col){}
    int line;
    int col;
    std::string stringToInsert;
    bool operator==(const EditSequenceElement other) const;
    bool operator<(const EditSequenceElement other) const;
  };

  class EditSequence {
  public:
    void addToInsert(int line, int col, std::string stringToInsert);
    void addToDelete(int startLine, int startCol, int endline, int endCol);
    void addToDelete(int startLine, int startCol, int endline, int endCol, OriginalFile const& originalFile);
    std::string toString();
    // checks whether an entry for (line,col) exists.
    int entryExists(int line, int col);
    bool toDelete(int line, int col);
    // provides string that is associated with (line,col) or empty string.
    std::list<std::string> getEntry(int line, int col);
    typedef std::multimap<EditSequenceElement,std::string> EditSequenceMapType;
    typedef std::set<EditSequenceElement> ToDeleteType;
  private:
    EditSequenceMapType _editSequenceMap;
    ToDeleteType _toDeleteSet;
  };

}

#endif
