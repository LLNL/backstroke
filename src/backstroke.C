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

#include "rose.h"

#include <iostream>
#include <vector>
#include <set>
#include <list>
#include <string>
#include <cmath>

#include <exception>

#include "limits.h"
#include "assert.h"

#include "SgNodeHelper.h"
#include "CommandLineOptions.h"
#include "CodeGenerator.h"
#include "Utility.h"

using namespace std;
using namespace Backstroke;

void ensureSingleInputFile(SgProject* root) {
  size_t numFiles=Backstroke::Utility::numberOfFiles(root);
  if(numFiles==0) {
    cerr<<"Error: no files available in project."<<endl;
    exit(1);
  } else if(numFiles>1) {
    cerr<<"Error: too many input files. Only one input file is allowed."<<endl;
    exit(1);
  }
}

int main(int argc, char* argv[]) {
  try {
    string requiredRoseVersion="0.9.9.87"; // previous: 0.9.8.5"
    if(!Backstroke::Utility::checkRoseVersionNumber(requiredRoseVersion)) {
      cerr<<"Error: installed version of ROSE "<<string(ROSE_PACKAGE_VERSION)<<" is too old. Required ROSE version is "<<requiredRoseVersion<<"."<<endl;
      return 1;
    }
    CommandLineOptions clo;
    clo.process(argc,argv);
    if(clo.isFinished()) {
      return 0;
    } else {
      string inputFileName;
      if(clo.optionPreprocessor()) {
        inputFileName=clo.preProcessTranslationUnit(argc,argv);
      }
      if(clo.optionStatusMessages())
        cout << "STATUS: Parsing and creating AST."<<endl;
      SgProject* root = frontend(argc,argv);
      ensureSingleInputFile(root);
      string fileName=Backstroke::Utility::singleSourceFileName(root);
      if(clo.optionRoseAstCheck()) {
        AstTests::runAllTests(root);
      }
      if(clo.optionStatusMessages())
        cout << "STATUS: Generating code."<<endl;
      Backstroke::CodeGenerator g(&clo);
      g.setFileNameWithPath(fileName);
      if(clo.optionPreprocessor()) {
        g.setOutputFileName(inputFileName); // overrides only filename not path
      }
      g.setOutputFileNamePrefix("backstroke_");
      if(clo.optionBackend()==1) {
        g.loadFile();
      }
      g.generateCode(root);
      if(clo.optionStatusMessages())
        cout << "STATUS: finished."<<endl;
    }
  } catch(std::exception& e) {
    cerr<<"std::exception:"<<e.what()<<"."<<std::endl;
  } catch(char* str) {
    cerr << "Exception raised: " << str << endl;
    return 1;
  } catch(...) {
    cerr << "Unknown exception raised. Bailing out. " <<endl;
    return 1;
  }
  return 0;
}
