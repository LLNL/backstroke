#include "backstroke/rtss.h"

#include <cstdio>
#include "TimeMeasurement.h"
#include "TimeMeasurement.C"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cassert>

#include REVERSE_TESTFILE

TimeDuration originalTime=0;
TimeDuration forwardTime=0;
TimeDuration forwardInitTime=0;
TimeDuration forwardFinTime=0;
TimeDuration reverseTime=0;
TimeDuration commitTime=0;
TimeMeasurement timer;
TimeMeasurement timerInit;
TimeMeasurement timerFin;
int mode;
int containerSize;

std::ofstream measurementsFile;

#pragma reversible exclude
void allocRunTimeStateStorage() {
  // init run time system
  switch(mode) {
  case 0:
    xpdes::initializeRTSS(); // default: using no pre-allocation
    break;
  case 1:
    xpdes::initializeRTSS(200000); // faster: using pre-allocation 
    break;
  default: std::cerr<<"Error: unknown backstroke run time state storage mode."<<std::endl;
    exit(1);
  }
}

#pragma reversible exclude
void deallocRunTimeStateStorage() {
  xpdes::finalizeRTSS();
}

#pragma reversible exclude
void originalCode(int num,int numOps) {
  srand(0);
  allocRunTimeStateStorage();
  State* state=new State();
  state->numOps=numOps;
  assert(containerSize>=0);
  state->containerSize=containerSize;
  timer.start();
  for(int i=0;i<num;i++) {
    //std::cout<<"Event-o: "<<i<<std::endl;
    event(state);
  }
  timer.stop();
  originalTime=timer.getTimeDuration();
  measurementsFile<<num
                  <<","<<originalTime.milliSeconds()
    ;
  std::cout<<"Original: "<<num<<" events, time: "<<originalTime.milliSeconds()<<" ms"<<std::endl;
  //state->cleanup();
  //xpdes::reset();
  delete state;
  deallocRunTimeStateStorage();
}

#pragma reversible exclude
void forwardReverseCode(int num, int numOps) {
  srand(0);
  forwardInitTime=0.0;
  forwardTime=0.0;
  forwardFinTime=0.0;
  reverseTime=0.0;

  allocRunTimeStateStorage();
  State* state=new State();
  state->numOps=numOps;
  state->containerSize=containerSize;
  for(int i=0;i<num;i++) {
    timerInit.start();
    xpdes::beginForwardEvent();
    timerInit.stop();
    forwardInitTime+=timerInit.getTimeDuration();
    
    timer.start();
    //std::cout<<"Event-f: "<<i<<std::endl;
    event(state);
    timer.stop();
    forwardTime+=timer.getTimeDuration();
    
    timerFin.start();
    xpdes::endForwardEvent();
    timerFin.stop();
    forwardFinTime+=timerFin.getTimeDuration();
  }
  measurementsFile
    <<","<<forwardInitTime.milliSeconds()
    <<","<<forwardTime.milliSeconds()
    <<","<<forwardFinTime.milliSeconds()
    ;
  std::cout<<"Forward : "<<num<<" events,"
      <<" time: "<<forwardTime.milliSeconds()<<" ms"
      <<" init: "<<forwardInitTime.milliSeconds()<<" ms"
      <<" fin: "<<forwardFinTime.milliSeconds()<<" ms"<<std::endl;
  timer.start();
  for(int i=0;i<num;i++) {
    //std::cout<<"Event-r: "<<i<<std::endl;
    xpdes::reverseEvent();
  }
  timer.stop();
  reverseTime=timer.getTimeDuration();

  measurementsFile<<","<<reverseTime.milliSeconds();
  std::cout<<"Reverse : "<<num<<" events, time: "<<reverseTime.milliSeconds()<<" ms"<<std::endl;

  //xpdes::reset();
  delete state;
  deallocRunTimeStateStorage();
}

#pragma reversible exclude
void forwardCommitCode(int num,int numOps) {
  srand(0);
  commitTime=0.0;
  allocRunTimeStateStorage();
  State* state=new State();
  state->numOps=numOps;
  state->containerSize=containerSize;
  //timer.start();
  for(int i=0;i<num;i++) {
    xpdes::beginForwardEvent();
    //std::cout<<"Event-f: "<<i<<std::endl;
    event(state);
    xpdes::endForwardEvent();
  }
  //timer.stop();
  //forwardTime=timer.getTimeDuration();
  //std::cout<<"Forward : "<<forwardTime.milliSeconds()<<" ms"<<std::endl;
  timer.start();
  for(int i=0;i<num;i++) {
    //std::cout<<"Event-c: "<<i<<std::endl;
    xpdes::commitEvent();
  }
  timer.stop();
  commitTime=timer.getTimeDuration();
  measurementsFile<<","<<commitTime.milliSeconds();
  std::cout<<"Commit  : "<<num<<" events, time: "<<commitTime.milliSeconds()<<" ms"<<std::endl;
  //state->cleanup();

  //state->cleanup();
  // performing clean up on allocated data structures. cleanup is not
  // part of the benchmark (and therfore not includeded in
  // measurements)
  {
    xpdes::beginForwardEvent();
    //MYREVERSE::delete_array(state->container);
    //MYREVERSE::event(state);
    state->cleanup();
    xpdes::endForwardEvent();
    xpdes::commitEvent();
  }

  delete state;
  deallocRunTimeStateStorage();
}

#pragma reversible exclude
int main(int argc, char* argv[]) {
  measurementsFile.open("measurements.txt");
  if(argc!=5) {
    std::cout<<"Error: wrong number of arguments."<<std::endl;
    std::cout<<"Usage: "<<argv[0]<<" NUMEVENTS MODE NUMOPS CONTAINERSIZE"<<std::endl;
    std::cout<<"       where MODE is 0 (default-alloc)"<<std::endl;
    std::cout<<"                     1 (pre-alloc)"<<std::endl;
    exit(1);
  }
  std::string numstring=std::string(argv[1]);
  std::string modestring=std::string(argv[2]);
  std::string numopsstring=std::string(argv[3]);
  std::string containerSizeString=std::string(argv[4]);

  int numEvents;
  std::stringstream is1(numstring); is1 >> numEvents;
  int mode0;
  std::stringstream is2(modestring); is2 >> mode0;
  mode=mode0; // use global mode variable
  int numops;
  std::stringstream is3(numopsstring); is3 >> numops;
  int containerSize0;
  std::stringstream is4(containerSizeString); is4 >> containerSize0;
  containerSize=containerSize0; // use global variable
  assert(containerSize>=0);
  allocRunTimeStateStorage();

  for(int i=0;i<=numops;i+=25) {
    measurementsFile<<i<<",";
    std::cout<<"BENCH: NUMOPS: "<<i<<std::endl;
    originalCode(numEvents,i);
    forwardReverseCode(numEvents,i);
    forwardCommitCode(numEvents,i);
    //    forwardReverseCode(numEvents,i); // do it again for testing
    //forwardCommitCode(numEvents,i); // do it again for testing

    double totalForwardTime=(forwardInitTime.milliSeconds()+forwardTime.milliSeconds()+forwardFinTime.milliSeconds());
    double penaltyFactor=totalForwardTime/originalTime.milliSeconds();
    measurementsFile<<","<<penaltyFactor;
    measurementsFile<<","<<totalForwardTime;
    std::cout<<"Penalty: "<<penaltyFactor<<std::endl;

    measurementsFile<<std::endl;
  }
  measurementsFile.close();
  return 0;
}
