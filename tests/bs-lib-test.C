#include "backstroke/rtss.h"
#include <iostream>
#include <cassert>

int main() {
  int catchCounter=0;
  int* a=new int();
  try {
    xpdes::initializeRTSS();
    xpdes::beginForwardEvent();
    xpdes::avpushT(*a);
    xpdes::endForwardEvent();
    xpdes::reverseEvent();
    
    // try to reverse non existing event
    xpdes::reverseEvent();
  } catch(Backstroke::Exception& e) {
    std::cout<<"PASS (testing Backstroke exception: "<<e.what()<<")"<<std::endl;
    catchCounter++;
  }
  try {
    xpdes::beginForwardEvent();
    xpdes::avpushT(*a);
    xpdes::endForwardEvent();
    xpdes::commitEvent();
    // try to reverse non existing event
    xpdes::commitEvent();
  }
  catch(Backstroke::Exception& e) {
    std::cout<<"PASS (testing Backstroke exception: "<<e.what()<<")"<<std::endl;
    catchCounter++;
  }
  xpdes::finalizeRTSS();

  assert(catchCounter==2);
  return 0;
}
