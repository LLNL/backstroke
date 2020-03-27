#!/bin/bash
# Author: Markus Schordan, 2015.

#: ${BS_SOURCE_PATH:=.}
: ${TOOL1:=g++}
: ${TOOL2:=backstroke}

BS_INPUT_LANG_STANDARD=c++14
BS_INPUT_LANG_STANDARD_OPTION=-std=c++14
BS_OUTPUT_LANG_STANDARD=c++14
BS_INPUT_LANG_STANDARD_OPTION=-std=c++14

STL_HEADERS_ALL="algorithm bitset complex deque exception fstream functional iomanip ios iosfwd iostream istream iterator limits list locale map memory new numeric ostream queue set sstream stack stdexcept streambuf string typeinfo utility valarray vector"

STL_HEADERS_PASS="algorithm bitset complex deque exception fstream functional ios iosfwd iostream istream iterator limits list map new numeric ostream queue set sstream stack stdexcept streambuf string typeinfo utility valarray vector"

STL_HEADERS_FAIL="memory iomanip locale"

rm -f *.pass.C *.fail.C *.o *.pp.C *.ti

GREEN="\033[30;1m"   # Green
RED="\033[31;1m"   # Red
COLOREND="\033[0m"
REDFAIL="${RED}FAIL${COLOREND}"

# the bs runtime lib is not used/linked with these tests yet.
#BS_INCLUDE=-I$BACKSTROKE_RUNTIMELIB
BS_INCLUDE=
BS_BACKEND_NR=1
BS_BACKEND=--backend=${BS_BACKEND_NR}
BS_ACCESS_MODE="--access-mode=1"
BACKSTROKE=../src/backstroke
BS_RTLIB_INCLUDE_OPTIONS=-I../src/rtss
BS_STL_SUPPORT=
BS_RESTRICTIONS=--ignore-unions
BS_LIB_INCLUDE_OPTION_CHECK="-include backstroke/rtss.h"
BS_LIB_INCLUDE_OPTION="--rtss-header"
#GCC_OPTIONS="-fabi-version=6" # only required for g++ 4.9
GCC_OPTIONS="-fpermissive"

echo "BS_RTLIB_INCLUDE_OPTIONS: ${BS_RTLIB_INCLUDE_OPTIONS}"

echo "-----------------------------------------------------------------"
echo "STL TEST: header transformation and compile test (no run tests)"
echo "C++ input/output language standard: ${BS_INPUT_LANG_STANDARD}/${BS_OUTPUT_LANG_STANDARD}"
echo "-----------------------------------------------------------------"
echo "Note: using g++ options: $GCC_OPTIONS"
echo "Unsupported C++ constructs: unions, bitfields."
echo "Unsupported STL headers: $STL_HEADERS_FAIL"
echo "Supported   STL headers: $STL_HEADERS_PASS"
echo "-----------------------------------------------------------------"

for header in $STL_HEADERS_PASS; do
  printf "TESTING: %-17s: " "$header"
  printf "#include <$header>\nint main(){ return 0; }\n" > test_${header}.C

  g++ $BS_INPUT_LANG_STANDARD_OPTION -E -P test_${header}.C ${BS_RTLIB_INCLUDE_OPTIONS} ${BS_LIB_INCLUDE_OPTION_CHECK} > test_${header}.check.C 
  #${BACKSTROKE} -std=$BS_INPUT_LANG_STANDARD -edg:E -edg:P test_${header}.C ${BS_RTLIB_INCLUDE_OPTIONS} ${BS_LIB_INCLUDE_OPTION}  > test_${header}.pp.C 

  LOC=`wc -l test_${header}.check.C| cut -f1 -d' '`
  printf "%6s LOC : " "$LOC"
  # use a sub shell and redirect coredump output of subshell to /dev/null
  {
  $BACKSTROKE --no-preprocessor $BS_RTLIB_INCLUDE_OPTIONS $BS_RESTRICTIONS $BS_BACKEND $BS_ACCESS_MODE $BS_INCLUDE --no-transform test_${header}.C $BS_INPUT_LANG_STANDARD_OPTION &> /dev/null
  } > /dev/null 2>&1
  if [ $? -eq 0 ]; then
      echo -n "PASS " # 0
  {
  $BACKSTROKE --preprocessor $BS_LIB_INCLUDE_OPTION $BS_RTLIB_INCLUDE_OPTIONS $BS_RESTRICTIONS $BS_BACKEND $BS_ACCESS_MODE $BS_INCLUDE --stats-csv-file=test_${header}.csv test_${header}.C $BS_INPUT_LANG_STANDARD_OPTION &> /dev/null
  } > /dev/null 2>&1
  if [ $? -eq 0 ]; then
      cp test_${header}.C.pp.C $header.pass.C
      echo -n "PASS" # 1
      if [ -e backstroke_test_${header}.C ]
      then
          cp backstroke_test_${header}.C backstroke_test_$header.t1.pass.C
          ${TOOL1} $BS_OUTPUT_LANG_STANDARD_OPTION $BS_RTLIB_INCLUDE_OPTIONS backstroke_test_${header}.C -w -Wfatal-errors $BS_STL_SUPPORT $GCC_OPTIONS #> /dev/null 2>&1
           if [ $? -eq 0 ]; then
              cp backstroke_test_${header}.C backstroke_test_$header.t2.pass.C
              echo -n " PASS : 100.00%" # 2
              echo -n " "
              echo -n `cat test_${header}.csv` | tr "\n" " "
          else
              # determine line number of error
              ERROR_LINE1=`${TOOL1} ${BS_OUTPUT_LANG_STANDARD_OPTION} ${BS_RTLIB_INCLUDE_OPTIONS} backstroke_test_${header}.pp.C -w -Wfatal-errors ${BS_STL_SUPPORT} 2>&1 | egrep backstroke_test_${header}.pp.C:[0-9]*:[0-9] | cut -f2 -d:`
              if [[ -z "${ERROR_LINE1// }" ]]
              then
                  # rerun and check for linker errors
                  ERROR_LINE2=`${TOOL1} ${BS_OUTPUT_LANG_STANDARD_OPTION} ${BS_RTLIB_INCLUDE_OPTIONS} backstroke_test_${header}.pp.C -w -Wfatal-errors ${BS_STL_SUPPORT} 2>&1 | egrep "undefined reference" | wc -l`
                  if [[ -z "${ERROR_LINE2// }" ]]
                  then
                      ERROR_LINE=" : ${RED}UNKNOWN ERROR${COLOREND}"
                  else
                      echo -en " ${RED}FAIL${COLOREND} :   0.00% (${RED}LINK${COLOREND} ERRORS:$ERROR_LINE2)"
                  fi
              else
                  ERROR_PERCENTAGE=`echo "scale=2; ${ERROR_LINE1}*100/${LOC}" | bc`
                  echo -en " ${RED}FAIL${COLOREND}"
                  printf " : %6s%% (LINE:%s)" "$ERROR_PERCENTAGE" "$ERROR_LINE1" # 2
                  echo -n " [ ${TOOL1} $BS_OUTPUT_LANG_STANDARD_OPTION backstroke_test_${header}.pp.C -w -Wfatal-errors $BS_STL_SUPPORT]"
              fi
                      
              cp backstroke_test_${header}.C backstroke_test_$header.t2.fail.C
          fi            
      else
          echo -n " [no file generated!] "
      fi
  else
      cp test_${header}.C.pp.C test_$header.fail.C
      if [ -e backstroke_test_${header}.C ]
      then
          cp backstroke_test_${header}.C backstroke_test_$header.t1.fail.C
      fi
         echo -en "${RED}FAIL ----${COLOREND}" # 1
  fi
  else
      echo -en "${RED}FAIL ---- ----${COLOREND}" # 0
      echo -n " [ $BACKSTROKE $BS_RTLIB_INCLUDE_OPTIONS $BS_RESTRICTIONS $BS_BACKEND $BS_ACCESS_MODE $BS_INCLUDE --no-transform test_${header}.pp.C -std=$BS_INPUT_LANG_STANDARD]"


  fi
  echo
done

passnum1=`ls -1 *.t1.pass.C 2> /dev/null | wc -l`
failnum1=`ls -1 *.t1.fail.C 2> /dev/null | wc -l`
passnum2=`ls -1 *.t2.pass.C 2> /dev/null | wc -l`
failnum2=`ls -1 *.t2.fail.C 2> /dev/null | wc -l`

echo "-----------------------------------------------------------------"
echo -e "PASS/FAIL                                     $passnum1/${RED}$failnum1${COLOREND} $passnum2/${RED}$failnum2${COLOREND}"
echo "-----------------------------------------------------------------"
