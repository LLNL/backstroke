
#include <list>
//#include <vector>
//#include <set>
// --------------------------------------

#ifdef USE_MAIN
#include <cstdlib>
#else
//extern int rand();
#endif

//#include <cstdio>
#include <cassert>

class State {
public:
  State():iter(0),list(0),numOps(50),maxlen(250000),containerSize(50000){}
  int iter;
  std::list<int>* list;
  //std::vector<int>* vector;
//std::set<int>* set;
  int numOps;
  int maxlen;
  int containerSize;

  void cleanup() {
    //delall_iter(list);
  }
};

#include <iostream>

void event(State* s) {
  int size=s->containerSize;
  if(s->list==0) {
    s->list=new std::list<int>();
  } else {
    int n=s->numOps;
    for(int i=0;i<n;++i) {
      //int r=rand();
      int r=10;
      s->list->push_back(r);
      s->list->push_back(r);
      s->list->pop_front();
    }
  }
#if 0
  if(s->vector==0) {
    s->vector=new std::vector<int>(size);
  } else {
    int n=s->numOps;
    for(int i=0;i<n;++i) {
      //int r=rand();
      int r=10;
//(*s->vector)[0]=r;
      s->vector->push_back(r);
//s->list->push_back(r);
    }
  }
#endif
#if 0
  if(s->set==0) {
    s->set=new std::set<int>();
  } else {
    int n=s->numOps;
    for(int i=0;i<n;++i) {
      //int r=rand();
      int r=10;
      s->set->insert(r);
    }
  }
#endif
}

#ifdef USE_MAIN
int main(int argc, char **argv) {
  State* s=new State();
  s->maxlen=100;
  for(int i=0;i<100000;i++) {
    event(s);
  }
  return 0;
}
#endif

