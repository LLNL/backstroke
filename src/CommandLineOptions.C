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

#include "CommandLineOptions.h"
#include "rose.h"
#include "assert.h"
#include <boost/program_options.hpp>
#include "SgNodeHelper.h"
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <cctype>

using namespace std;
using namespace Backstroke;

boost::program_options::variables_map args;  

Backstroke::CommandLineOptions::CommandLineOptions(): 
  _optionVersion(false),
  _optionRoseHelp(false),
  _optionTransform(true),
  _optionPreprocessor(true),
  _optionRoseAstCheck(false),
  _optionStatusMessages(false),
  _optionShowTransformationTrace(false),
  _optionAccessOperatorArrow(true),
  _optionTemplateCheck(false),
  _optionExperimental(false),
  _optionRTSSAccessMode(1),
  _optionBackend(1),
  _optionStats(false),
  _isFinished(false),
  _optionGenerateDefaultConstructor(false),
  _optionGenerateDefaultCopyConstructor(false),
  _optionGenerateDefaultAssignOp(true),
  _optionShowDefaultAssignOp(false),
  _optionOptLocalVar(true),
  _optionBackstrokeLibraryHeader(false),
  _optionUnion(true),
  _optionBitField(true),
  _optionDebug(false)
{
}

bool
Backstroke::CommandLineOptions::isFinished() {
  return _isFinished;
}

bool
Backstroke::CommandLineOptions::isError() {
  // there is no error detection yet, as unrecognized arguments are passed to ROSE (e.g. -I)
  return false;
}

bool
Backstroke::CommandLineOptions::optionRoseAstCheck() {
  return _optionRoseAstCheck;
}

bool
Backstroke::CommandLineOptions::optionTransform() {
  return _optionTransform;
}

bool
Backstroke::CommandLineOptions::optionPreprocessor() {
  return _optionPreprocessor;
}

bool
Backstroke::CommandLineOptions::optionStatusMessages() {
  return _optionStatusMessages;
}

bool
Backstroke::CommandLineOptions::optionTemplateCheck() {
  return _optionTemplateCheck;
}

bool
Backstroke::CommandLineOptions::optionExperimental() {
  return _optionExperimental;
}

bool
Backstroke::CommandLineOptions::optionShowTransformationTrace() {
  return _optionShowTransformationTrace;
}

bool
Backstroke::CommandLineOptions::optionAccessOperatorArrow() {
  return _optionAccessOperatorArrow;
}

int
Backstroke::CommandLineOptions::optionRTSSAccessMode() {
  return _optionRTSSAccessMode;
}

int
Backstroke::CommandLineOptions::optionBackend() {
  return _optionBackend;
}

bool
Backstroke::CommandLineOptions::optionStats() {
  return _optionStats;
}

std::string
Backstroke::CommandLineOptions::optionCsvStatsFileName() {
  return _optionCsvStatsFileName;
}

bool
Backstroke::CommandLineOptions::optionGenerateDefaultConstructor() {
  return _optionGenerateDefaultConstructor;
}

bool
Backstroke::CommandLineOptions::optionGenerateDefaultCopyConstructor() {
  return _optionGenerateDefaultCopyConstructor;
}

bool
Backstroke::CommandLineOptions::optionGenerateDefaultAssignOp() {
  return _optionGenerateDefaultAssignOp;
}

bool
Backstroke::CommandLineOptions::optionShowDefaultAssignOp() {
  return _optionShowDefaultAssignOp;
}

bool
Backstroke::CommandLineOptions::optionOptLocalVar() {
  return _optionOptLocalVar;
}

bool
Backstroke::CommandLineOptions::optionBackstrokeLibraryHeader() {
  return _optionBackstrokeLibraryHeader;
}

bool
Backstroke::CommandLineOptions::optionUnion() {
  return _optionUnion;
}

bool
Backstroke::CommandLineOptions::optionBitField() {
  return _optionBitField;
}

bool
Backstroke::CommandLineOptions::optionDebug() {
  return _optionDebug;
}

void
Backstroke::CommandLineOptions::process(int argc, char* argv[]) {
  if(argc==1) {
    cout << "Error: wrong command line options."<<endl;
    exit(1);
  }
  // Command line option handling.
  namespace po = boost::program_options;
  boost::program_options::options_description visible("Supported options");
  
  visible.add_options()
    ("status", "prints status messages while processing the input file.")
    ("rtss-header", "adds option -include \"backstroke/rtss.h\" when preprocessing input.")
    ("no-preprocessor", "turns off the pre-processing of the input file.")
    ("no-optimization", "turns off optimization. Does not optimize the use of local (auto) variables.")
    ("no-transform", "only parses and generates the original code, does not perform any transformation (only useful for testing).")
    ("ignore-unions", "does not generate assignment operator for unions")
    ("trace", "shows a trace of all transformations performed to obtain reversible code.")
    ("stats", "print transformation statistics.")
    ("stats-csv-file",po::value< string >(), "write transformation statistics in CSV format to file [arg].")
    ("help,h", "produces this help message.")
    ("version,v", "displays the backstroke version number.")
    ;

  boost::program_options::options_description hidden("Hidden options");
  hidden.add_options()
    ("show-default-ops", "lists classes and its non-implemented default operators.")
    ("opt-local-var", "optimizes the use of local (auto) variables [default]")
    ("no-opt-local-var", "optimizes the use of local (auto) variables [default]")
    ("preprocessor", "turns on the pre-processing of the input file. [default]")
    ("no-rtss-header", "does not generate backstroke library include directive when preprocessing the input file. [default]")
    ("access-operator-arrow", "generates operator '->' for accessing the RTSS member functions in transformed code. [default]")
    ("access-operator-dot", "generates operator '.' (instead of '->') for accessing the backstroke library member functions in transformed code.")
    ("no-assign-op","do not generate reversible assignment operators (only useful for testing).")
    ("default-ops", "generates default operator(s)")
    ("no-default-ops", "does not generate default operator(s)")
    ("ignore-bitfields", "ignores assignments to bitfields")
    ("copy-constr","generate reversible copy constructors")
    ("no-copy-constr","dot not generate reversible copy constructors. [default]")
    ("experimental", "enables experimental features.")
    ("template-check","checks for existing templates.")
    ("backend",po::value< int >(),"select backend [1-3]")
    ("access-mode",po::value< int >(),"select the RTSS access mode [1-2]")
    ("rose-ast-check", "performs consistency test of ROSE AST.")
    ("debug","turn on debug output")
    ;
  boost::program_options::options_description all("Supported options");
  all.add(visible).add(hidden);

  po::store(po::command_line_parser(argc, argv).options(all).allow_unregistered().run(), args);
  po::notify(args);
  
  if (args.count("help")) {
    cout << "backstroke <filename> [OPTIONS]"<<endl;
    cout << visible << "\n";
    _isFinished=true;
  }
  if (args.count("rose-help")) {
      argv[1] = strdup("--help");
      _optionRoseHelp=true;
      _isFinished=true;
  }
  if (args.count("rose-ast-check")) {
    _optionRoseAstCheck=true;
  }
  if (args.count("no-transform")) {
    _optionTransform=false;
  }
  if (args.count("preprocessor")) {
    _optionPreprocessor=true;
  }
  if (args.count("no-preprocessor")) {
    _optionPreprocessor=false;
  }
  if (args.count("status-messages")) {
    _optionStatusMessages=true;
  }
  if (args.count("template-check")) {
    _optionTemplateCheck=true;
  }
  if (args.count("experimental")) {
    _optionExperimental=true;
  }
  if (args.count("copy-constr")) {
    _optionGenerateDefaultConstructor=true;
    _optionGenerateDefaultCopyConstructor=true;
  }
  if (args.count("no-copy-constr")) {
    _optionGenerateDefaultConstructor=false;
    _optionGenerateDefaultCopyConstructor=false;
  }
  if (args.count("no-assign-op")) {
    _optionGenerateDefaultAssignOp=false;
  }
  if (args.count("show-assign-op")) {
    _optionShowDefaultAssignOp=true;
  }
  if (args.count("opt-local-var")) {
    _optionOptLocalVar=true;
  }
  if (args.count("no-opt-local-var")) {
    _optionOptLocalVar=false;
  }
  if (args.count("no-optimization")) {
    // place here all options to deactivate any optimization
    _optionOptLocalVar=false;
  }
  if (args.count("rtss-header")) {
    _optionBackstrokeLibraryHeader=true;
  }
  if (args.count("no-rtss-header")) {
    _optionBackstrokeLibraryHeader=false;
  }
  if (args.count("ignore-unions")) {
    _optionUnion=false;
  }
  if (args.count("ignore-bitfields")) {
    _optionBitField=false;
  }
  if (args.count("debug")) {
    _optionDebug=true;
  }
  if (args.count("backend")) {
    _optionBackend=args["backend"].as<int>();
    if(_optionBackend!=1) {
      cerr<<"Error: option backend: only value 1 is allowed."<<endl;
      _isFinished=true;
    }
  }
  if (args.count("access-mode")) {
    _optionRTSSAccessMode=args["access-mode"].as<int>();
    if(_optionRTSSAccessMode<1||_optionRTSSAccessMode>3) {
      cerr<<"Error: option access-mode: only values between 1 and 3 are allowed."<<endl;
      _isFinished=true;
    }
  }

  // check supported backend/access mode option
  if( (_optionBackend==1 && _optionRTSSAccessMode==3)
      || (_optionBackend==2 && _optionRTSSAccessMode==1)
      ) {
    cerr<<"Error: access mode "<<_optionRTSSAccessMode<<" not supported for backend "<<_optionBackend<<"."<<endl;
    exit(1);
  }

  if(optionBackend()!=1) {
    _optionTemplateCheck=true;
  }
  
  if (args.count("version")) {
    _optionVersion=true;
    _isFinished=true;
    cout << "Version 2.1.4"<<endl;
    cout << "Written by Markus Schordan 2014-2019."<<endl;
  }
  if (args.count("trace")) {
    _optionShowTransformationTrace=true;
  }

  if (args.count("access-operator-arrow")&&args.count("access-operator-dot")) {
    cout<<"Error: only one of the two options --access-operator-arrow and --access-operator-dot can be used."<<endl;
    exit(1);
  }

  if (args.count("access-operator-arrow")) {
    _optionAccessOperatorArrow=true;
  }
  if (args.count("access-operator-dot")) {
    _optionAccessOperatorArrow=false;
  }
    
  if (args.count("stats")) {
    _optionStats=true;
  }

  if (args.count("stats-csv-file")) {
    _optionCsvStatsFileName=args["stats-csv-file"].as<string>();
  }

  // TODO: duplication of names to be avoided
  vector<string> optionsList0={
    "rtss-header",
    "no-rtss-header",
    "no-optimization",
    "preprocessor",
    "no-preprocessor",
    "no-transform",
    "trace",
    "stats",
    "stats-csv-file",
    // hidden options
    "opt-local-var",
    "no-opt-local-var",
    "no-assign-op",
    "access-operator-arrow",
    "access-operator-dot",
    "show-default-ops",
    "backend",
    "access-mode",
    "experimental",
    "template-check",
    "rose-ast-check",
    "debug",
    "default-ops",
    "no-default-ops",
    "ignore-unions",
    "ignore-bitfields",
    "copy-constr",
    "no-copy-constr",
    "rose-ast-check",
  };
  // clean up string-options in argv to not confuse ROSE frontend
  for (int i=1; i<argc; ++i) {
    for (string clOption : optionsList0) {
      if (string(argv[i]).find("--"+clOption)==0) {
        argv[i] = strdup("");
      }
    }
  }
}

std::string Backstroke::CommandLineOptions::preProcessTranslationUnit(int argc, char* argv[]) {
  string inputFileName;
  string intermediateFileName;
  if(this->optionStatusMessages()) {
    cout << "STATUS: Preprocessing input file."<<endl;
  }

  // determine compiler name
  FILE *lsofFile_p = popen("rose-config cxx", "r");
  if (!lsofFile_p)
  {
    cerr<<"Error: could not determine compiler for pre-processing. Could not find rose-config."<<endl;
    exit(1);
  }
  char buffer[1024];
  char *line_p = fgets(buffer, sizeof(buffer), lsofFile_p);
  pclose(lsofFile_p);

  int inputFileNameArgc=-1;
  string compiler=string(line_p);
  compiler.erase(remove_if(compiler.begin(), compiler.end(), ::isspace), compiler.end()); // remove whitespace
  if(this->optionStatusMessages()) {
    cout<<"STATUS: using compiler "<<compiler<<" for pre-processing."<<endl;
  }
  // rename input file to name of preprocessed file in argv
  string preprocessor=compiler+" -E -P ";
  // add backstroke library include header if requested
  if(this->optionBackstrokeLibraryHeader()) {
    preprocessor+=" -include \"backstroke/rtss.h\"";
  }
  for(int i=1;i<argc;i++) {
    string s=string(argv[i]);
    // do not copy this option for the pre-processor command line
    if(SgNodeHelper::isPrefix("--edg:no_warnings",s)) {
      continue;
    }
    if(SgNodeHelper::isPrefix("-o",s)
       ||SgNodeHelper::isPrefix("-I",s)
       ||SgNodeHelper::isPrefix("-finline-limit",s)
       ) {
      if(s.size()==2) {
        // copy this and following
        preprocessor+=(string(" "))+string(argv[i]);
        i+=1;
      }
    }
    if(SgNodeHelper::isPrefix("-",s) // other options
       ||s.size()==0 // erased backstroke option
       ) {
      // just copy
    } else {
      // found file
      inputFileNameArgc=i;
      inputFileName=argv[inputFileNameArgc];
      intermediateFileName=inputFileName+".pp.C";  // cannot use .ii because ROSE does not accept it.
      preprocessor+=" -o "+intermediateFileName+" "+inputFileName;
      continue;
    }
    preprocessor+=" "+string(argv[i]);
  } // end of command line options loop
  if(optionStatusMessages()) cout<<"STATUS: preprocessor command line: "<<preprocessor<<endl;
  int returnCode;
  returnCode=system(preprocessor.c_str());
  if(returnCode!=0) {
    cerr<<"Error: Could not preprocess file "<<inputFileName<<endl;
    exit(1);
  }
  // replace original name with intermediate file name
  argv[inputFileNameArgc]= strdup((char*)intermediateFileName.c_str()); // duplication necessary to remain valid until frontend is called.

  return inputFileName;
}
