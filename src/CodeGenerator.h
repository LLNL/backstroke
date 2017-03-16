#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

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

class SgProject;
class SgFunctionDefinition;

#include <vector>
#include <string>
#include "CommandLineOptions.h"
#include "PragmaMap.h"
#include "TransformationSequence.h"

namespace Backstroke {
  class CodeGenerator {
  public:
    CodeGenerator();
    CodeGenerator(CommandLineOptions* clo);
    void setCommandLineOptions(CommandLineOptions* clo);
    void generateCode(SgProject* root);
    void setFileNameWithPath(std::string fileNameWithPath);
    void setOutputFileName(std::string fileName);
    void setOutputFileNamePrefix(std::string);
    void loadFile();
    void saveEditedFileAs();
    void initNameQualification(SgProject* project);
    std::string originalFileContent();
  private:
    void init();
    std::string getFileNameWithPath();
    std::string getFileName();
    std::string getFilePath();
    std::string getOutputFileNameWithPath();
    std::string getOutputFileNamePrefix();
  public:
    std::string getOutputFileName();
  private:
    void applyEditTransformationSequence();
    //void forwardCodeTransformation(SgFunctionDefinition* funDef);
    void forwardCodeTransformation(SgNode* funDef);
    void checkTemplateInstantiations(SgProject* root);
    void markTemplateInstantiationToBeUnparsed(SgNode* node);
    /* marks the entire subtree of all template instantation nodes to
       be unparsed. */
    bool isUnmarkedTemplateNodeToBeUnparsed(SgNode* node);
    int markAllTemplateInstantiationsToBeUnparsed(SgProject* root); 
    CommandLineOptions* _commandLineOptions;
    PragmaMap _pragmaMap;
    OriginalFile _originalFileContent;
    std::string _editedFileContent;
    TransformationSequence _transformationSequence;
    std::string _fileName;
    std::string _filePath;
    std::string _fileNameWithPath;
    std::string _outputFileName;
    std::string _outputFileNameWithPath;
    std::string _outputFileNamePrefix;
  };
};

#endif
