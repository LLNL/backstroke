#ifndef COMMAND_LINED_OPTIONS_H
#define COMMAND_LINED_OPTIONS_H

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

#include <string>

namespace Backstroke {

  class CommandLineOptions {
  public:
    CommandLineOptions();
    void process(int argc, char* argv[]);
    // returns file name of translation unit and preprocesses file
    // the intermediata file has name <inputfilename>.pp.C - will become .ii when ROSE supports it.
    std::string preProcessTranslationUnit(int argc, char* argv[]);
    bool optionRoseAstCheck();
    bool optionTransform();
    bool optionStatusMessages();
    bool optionShowTransformationTrace();
    bool optionAccessOperatorArrow();
    bool optionTemplateCheck();
    bool optionPreprocessor();
    bool optionExperimental();
    int  optionRTSSAccessMode();
    int  optionBackend();
    bool optionStats();
    bool optionGenerateDefaultConstructor();
    bool optionGenerateDefaultCopyConstructor();
    bool optionGenerateDefaultAssignOp();
    bool optionShowDefaultAssignOp();
    bool optionOptLocalVar();
    bool optionBackstrokeLibraryHeader();
    bool optionUnion();
    bool optionBitField();
    bool optionDebug();
    std::string optionCsvStatsFileName();
    bool isFinished();
    bool isError();
  private:
    bool _optionVersion;
    bool _optionRoseHelp;
    bool _optionTransform;
    bool _optionPreprocessor;
    bool _optionRoseAstCheck;
    bool _optionStatusMessages;
    bool _optionShowTransformationTrace;
    bool _optionAccessOperatorArrow;
    bool _optionTemplateCheck;
    bool _optionExperimental;
    int _optionRTSSAccessMode;
    int _optionBackend;
    bool _optionStats;
    std::string _optionCsvStatsFileName;
    bool _isFinished;
    bool _optionEliminateGlobalDecls;
    bool _optionGenerateDefaultConstructor;
    bool _optionGenerateDefaultCopyConstructor;
    bool _optionGenerateDefaultAssignOp;
    bool _optionShowDefaultAssignOp;
    bool _optionOptLocalVar;
    bool _optionBackstrokeLibraryHeader;
    bool _optionUnion;
    bool _optionBitField;
    bool _optionDebug;
  };

};

#endif
