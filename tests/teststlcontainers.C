#include "backstroke/rtss.h"

#include <cstdio>

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cassert>

#include <deque>
#include <queue>
#include <map>
#include <list>
#include <vector>

#include "TimeMeasurement.h"
#include "TimeMeasurement.C"

#define TEST_OUTPUT

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

template<typename ContainerType>
class State {
public:
  State():iter(0),container(0),numOps(50),maxlen(250000),containerSize(50000){}
  int iter;
  ContainerType* container;
  int numOps;
  int maxlen;
  int containerSize;

  void cleanup() {
  }
};

#pragma reversible exclude
template<typename ContainerType>
void init(State<ContainerType>* s) {
  s->container=new ContainerType();
}

#pragma reversible exclude
template<typename ContainerType>
void print_container(State<ContainerType>* s) {
  for(typename ContainerType::iterator i=s->container->begin();
      i!=s->container->end();
      ++i ) {
    std::cout<<(*i)<<" ";
  }
  std::cout<<std::endl;
}

// specialize for those containers that do not offer an iterator
template<> void print_container(State<std::queue<int> >* s) {
  std::cout<<"queue size: "<<s->container->size()<<": ";
  std::queue<int> t=*(s->container);
  int n=0;
  while(!t.empty()) {
    int w = t.front();
    std::cout << w << " ";
    t.pop();
    n++;
  }
  std::cout<<std::endl;
}

template<> void print_container(State<std::stack<int> >* s) {
  std::cout<<"stack size: "<<s->container->size()<<": ";
  std::stack<int> t=*(s->container);
  int n=0;
  while(!t.empty()) {
    int w = t.top();
    std::cout << w << " ";
    t.pop();
    n++;
  }
  std::cout<<std::endl;
}

template<> void print_container(State<std::map<int,int> >* s) {
  std::cout<<"map size: "<<s->container->size()<<": ";
  for(typename std::map<int,int>::iterator i=s->container->begin();
      i!=s->container->end();
      ++i ) {
    std::cout<<"("<<(*i).first<<","<<(*i).second<<")";
  }
  std::cout<<std::endl;
}

template<typename ContainerType>
void event(State<ContainerType>* s, int eventNr) {
  int n=s->numOps;
  s->container->push_front(eventNr);
  s->container->push_back(eventNr);
  s->container->pop_front();
} 
template<> void event(State<std::queue<int> >* s, int eventNr) {
  s->container->push(eventNr);
}
template<> void event(State<std::stack<int> >* s, int eventNr) {
  s->container->push(eventNr);
  s->container->pop();
  s->container->push(eventNr);
}
template<> void event(State<std::map<int,int> >* s, int eventNr) {
  (*(s->container))[eventNr]=eventNr;
}
template<> void event(State<std::vector<int> >* s, int eventNr) {
  s->container->push_back(eventNr);
  s->container->pop_back();
  s->container->push_back(eventNr);
}

template<> void event(State<std::string >* s, int eventNr) {
  (*s->container)=(*s->container)+"x";
}



#pragma reversible exclude
template<typename ContainerType>
void check(State<ContainerType>* s, int eventNr) {
  #ifdef TEST_OUTPUT
  std::cout<<"check: "<<s->container->size()-1<<"=?="<<eventNr<<std::endl;
  #endif
  assert(s->container->size()-1==eventNr);
  typename ContainerType::iterator i=s->container->begin();
  for(int e=0;e<eventNr;e++) {
#if USE_MAIN
    std::cout<<"check "<<e<<": "<<(*i)<<"=?="<<e<<std::endl;
#endif
    assert((*i)==e);
    ++i;
  }
}

#pragma reversible exclude
template<>
void check(State<std::stack<int> >* s, int eventNr) {
  // queue has not iterator, therefor the test is reduced to the size
  assert(s->container->size()-1==eventNr);
  std::stack<int> t=*(s->container);
  int n=0;
  while(!t.empty()) {
    t.pop();
    n++;
  }
  assert(n==s->container->size());
}

#pragma reversible exclude
template<>
void check(State<std::queue<int> >* s, int eventNr) {
  // queue has not iterator, therefor the test is reduced to the size
  assert(s->container->size()-1==eventNr);
  std::queue<int> t=*(s->container);
  int n=0;
  while(!t.empty()) {
    t.pop();
    n++;
  }
  assert(n==s->container->size());
}

template<>
void check(State<std::map<int,int> >* s, int eventNr) {
  assert(s->container->size()-1==eventNr);
}
template<>
void check(State<std::string>* s, int eventNr) {
#ifdef TEST_OUTPUT
  std::cout<<"containersize-1: "<<s->container->size()-1<<" eventNr:"<<eventNr<<std::endl;
#endif
  assert(s->container->size()-1==eventNr);
}

#ifdef USE_MAIN
int main(int argc, char **argv) {
  State<list<int> >* s=new State<list<int> >();
  s->maxlen=100;
  init<list<int> >(s);
  for(int i=0;i<10;i++) {
    event<list<int> >(s,i);
    check<list<int> >(s,i);
  }
  return 0;
}
#endif

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
template<typename ContainerType>
void originalCode(int num,int numOps) {
    srand(0);
  allocRunTimeStateStorage();
  State<ContainerType>* state=new State<ContainerType>();
  state->numOps=numOps;
  assert(containerSize>=0);
  state->containerSize=containerSize;
  init<ContainerType>(state);
  timer.start();
  for(int i=0;i<num;i++) {
    event<ContainerType>(state,i);
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
template<typename ContainerType>
void forwardReverseCode(int num, int numOps) {
  srand(0);
  forwardInitTime=TimeDuration(0.0);
  forwardTime=TimeDuration(0.0);
  forwardFinTime=TimeDuration(0.0);
  reverseTime=TimeDuration(0.0);

  allocRunTimeStateStorage();
  State<ContainerType>* state=new State<ContainerType>();
  state->numOps=numOps;
  state->containerSize=containerSize;
  init(state);
  for(int i=0;i<num;i++) {
    timerInit.start();
    xpdes::beginForwardEvent();
    timerInit.stop();
    forwardInitTime+=timerInit.getTimeDuration();
    
    timer.start();
    //std::cout<<"Event-f: "<<i<<std::endl;
    event<ContainerType>(state,i);
    timer.stop();
    forwardTime+=timer.getTimeDuration();
  
    timerFin.start();
    xpdes::endForwardEvent();
    timerFin.stop();
    forwardFinTime+=timerFin.getTimeDuration();
    check<ContainerType>(state,i);
#ifdef TEST_OUTPUT
    print_container<ContainerType>(state);
#endif
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
  for(int i=num-1;i>=0;--i) {
    //std::cout<<"Event-r: "<<i<<std::endl;
    check<ContainerType>(state,i);
#ifdef TEST_OUTPUT
    print_container<ContainerType>(state);
#endif
    xpdes::reverseEvent();
  }
  timer.stop();
  reverseTime=timer.getTimeDuration();

  measurementsFile<<","<<reverseTime.milliSeconds();
  std::cout<<"Reverse : "<<num<<" events, time: "<<reverseTime.milliSeconds()<<" ms"<<std::endl;

  //xpdes::reset();
  //delete state->container; (this is allocated in the forward-code and should be deleted by commit)
  delete state;
  deallocRunTimeStateStorage();
}

#pragma reversible exclude
template<typename ContainerType>
void forwardCommitCode(int num,int numOps) {
  srand(0);
  commitTime=0.0;
  allocRunTimeStateStorage();
  State<ContainerType>* state=new State<ContainerType>();
  state->numOps=numOps;
  state->containerSize=containerSize;
  init<ContainerType>(state);
  for(int i=0;i<num;i++) {
    xpdes::beginForwardEvent();
    event<ContainerType>(state,i);
    xpdes::endForwardEvent();
  }
  timer.start();
  for(int i=0;i<num;i++) {
    xpdes::commitEvent();
  }
  timer.stop();
  commitTime=timer.getTimeDuration();
  measurementsFile<<","<<commitTime.milliSeconds();
  std::cout<<"Commit  : "<<num<<" events, time: "<<commitTime.milliSeconds()<<" ms"<<std::endl;

  // performing clean up on allocated data structures. cleanup is not
  // part of the benchmark (and therfore not includeded in
  // measurements)
  {
    xpdes::beginForwardEvent();
    state->cleanup();
    xpdes::endForwardEvent();
    xpdes::commitEvent();
  }

  delete state;
  deallocRunTimeStateStorage();
}

template<typename ContainerType>
void performTest(int numEvents, int numops) {
  //for(int i=0;i<=numops;i+=25) {
  int i=0;
    measurementsFile<<i<<",";
    //    std::cout<<"BENCH: NUMOPS: "<<i<<std::endl;
    originalCode<ContainerType>(numEvents,i);
    forwardReverseCode<ContainerType>(numEvents,i);
    forwardCommitCode<ContainerType>(numEvents,i);
    double totalForwardTime=(forwardInitTime.milliSeconds()+forwardTime.milliSeconds()+forwardFinTime.milliSeconds());
    double penaltyFactor=totalForwardTime/originalTime.milliSeconds();
    measurementsFile<<","<<penaltyFactor;
    measurementsFile<<","<<totalForwardTime;
    //std::cout<<"Penalty: "<<penaltyFactor<<std::endl;
    measurementsFile<<std::endl;
    //}
}

void print_testing(std::string s) {
  std::cout<<"\n=== TESTING "<<s<<" === "<<std::endl;
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

#ifdef REVERSIBLE_STDLIB_DEPENDENT_TESTS
  print_testing("std::list<int>");
  performTest<std::list<int> >(numEvents,numops);
  print_testing("std::map<int,int>");
  performTest<std::map<int,int> >(numEvents,numops);
#if 0
  print_testing("std::string<int>");
  performTest<std::string>(numEvents,numops); // TODO
#endif
#endif
  print_testing("std::stack<int>");
  performTest<std::stack<int> >(numEvents,numops);
  print_testing("std::deque<int>"); 
  performTest<std::deque<int> >(numEvents,numops);
  print_testing("std::queue<int>");
  performTest<std::queue<int> >(numEvents,numops);
  print_testing("std::vector<int>");
  performTest<std::vector<int> >(numEvents,numops);

  measurementsFile.close();
  return 0;
}
