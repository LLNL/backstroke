#include <cstdio>

#include "backstroke/rtss.h"

#include "TimeMeasurement.h"
#include "TimeMeasurement.C"

#include <iostream>
#include <sstream>
#include <cassert>

using namespace std;

#include REVERSE_TESTFILE

TimeDuration originalTime;
TimeDuration forwardTime;
TimeDuration forwardInitTime;
TimeDuration forwardFinTime;
TimeDuration reverseTime;
TimeDuration commitTime;
TimeMeasurement timer;
TimeMeasurement timerInit;
TimeMeasurement timerFin;

void originalCode(int num) {
    State* state=new State();
    timer.start();
    for(int i=0;i<num;i++) {
      event(state);
    }
    timer.stop();
    originalTime=timer.getTimeDuration();
    cout<<"Original: "<<num<<" events, time: "<<originalTime.milliSeconds()<<" ms"<<endl;
}

void forwardReverseCode(int num) {
    State* state=new State();
    xpdes::initializeRTSS();
    timer.start();
    for(int i=0;i<num;i++) {
      timerInit.start();
      xpdes::beginForwardEvent();
      timerInit.stop();
      forwardInitTime+=timerInit.getTimeDuration();
      event(state);
      
      timerFin.start();
      xpdes::endForwardEvent();
      timerFin.stop();
      forwardFinTime+=timerFin.getTimeDuration();
    }
    timer.stop();
    forwardTime=timer.getTimeDuration();
    cout<<"Forward : "<<num<<" events, time: "<<forwardTime.milliSeconds()-forwardInitTime.milliSeconds()-forwardFinTime.milliSeconds()<<" ms"
        <<" init: "<<forwardInitTime.milliSeconds()<<" ms"
        <<" fin:"<<forwardFinTime.milliSeconds()<<" ms"<<endl;
    timer.start();
    for(int i=0;i<num;i++) {
      xpdes::reverseEvent();
    }
    timer.stop();
    reverseTime=timer.getTimeDuration();
    cout<<"Reverse : "<<num<<" events, time: "<<reverseTime.milliSeconds()<<" ms"<<endl;
    xpdes::finalizeRTSS();
}

void forwardCommitCode(int num) {
    State* state=new State();
    xpdes::initializeRTSS();
    timer.start();
    for(int i=0;i<num;i++) {
      xpdes::beginForwardEvent();
      event(state);
      xpdes::endForwardEvent();
    }
    timer.stop();
    forwardTime=timer.getTimeDuration();
    //cout<<"Forward : "<<forwardTime<<" ms"<<endl;
    timer.start();
    for(int i=0;i<num;i++) {
      xpdes::commitEvent();
    }
    timer.stop();
    commitTime=timer.getTimeDuration();
    cout<<"Commit  : "<<num<<" events, time: "<<commitTime.milliSeconds()<<" ms"<<endl;
    xpdes::finalizeRTSS();

}

int main(int argc, char* argv[]) {
  
  if(argc!=3) {
    cout<<"Error: wrong number of arguments."<<endl;
    cout<<"Usage: "<<argv[0]<<" NUM MODE"<<endl;
    cout<<"       where MODE is 0 (original)"<<endl;
    cout<<"                     1 (fwd+rev)"<<endl;
    cout<<"                  or 2 (fwd+commit)"<<endl;
    exit(1);
  }
  string numstring=string(argv[1]);
  string modestring=string(argv[2]);

  int num;
  stringstream is1(numstring); is1 >> num;
  int mode;
  stringstream is2(modestring); is2 >> mode;

  switch(mode) {
  case 0: originalCode(num); break;
  case 1: forwardReverseCode(num); break;
  case 2: forwardCommitCode(num); break;
  default:
    cout<<"Error: wrong mode."<<endl;
    return 1;
  }
  return 0;
}
