#ifndef RTSS_LIBRARY_H
#define RTSS_LIBRARY_H

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

namespace Backstroke {
  class RunTimeStateStore;
  class RunTimeLpStateStore;
}

// required for operatorDeleteT(T* mem, const std::nothrow_t& tag);
namespace std {
  struct nothrow_t;
}

// xpdes declarations MUST BE in this header file before the STL header includes
namespace xpdes {

  // this variable is defined in the backstroke library
  extern Backstroke::RunTimeStateStore* internalRTSS;

  // this variable is defined in the backstroke library
  extern Backstroke::RunTimeLpStateStore* rtlpss;
  
  void initializeRTSS();
  void initializeRTSS(int n);
  void finalizeRTSS();
  inline bool enableDataRecording();
  inline bool disableDataRecording();
  inline bool setDataRecording(bool);
  inline bool dataRecording();
  inline void beginForwardEvent();
  inline void endForwardEvent();
  inline void reverseEvent();
  inline void commitEvent();

  // TODO: with C++11 the next line can be removed
  typedef unsigned long size_t;

  template <typename T> T& avpushT(T& lhs);
  template <typename T> T*& avpushT(T*& lhs);
  template <typename T> T* registerAllocationT(T* mem);
  template <typename T> T* registerOperatorNewT(T* mem);
  template <typename T> T* allocateArrayT(size_t numberOfElements);
  template <typename T> void registerDeallocationT(T* p);
  template <typename T> void operatorDeleteT(T* mem);
  template <typename T> void operatorDeleteT(T* mem, const std::nothrow_t& tag);
  template <typename T> void registerArrayDeallocationT(T* deleteOperand);

}
// this included code will be instrumented with above xpdes function calls
// therefore the xpdes namespace is broken up here to allow for the declaration
// of the ross types for the following xpdes functions which use twlp.
#ifdef RTSS_LIBRARY_WITH_ROSS
#include "ross.h"
#endif

#ifdef RTSS_LIBRARY_WITH_ROSS
namespace Backstroke {
  class SimTime {
  public:
    SimTime();
    SimTime(tw_stime simTime);
    bool operator<(const SimTime& other);
#ifdef BS_DEBUG_PRINT
    std::string toString() { std::stringstream ss; ss<<_simTime.t; return ss.str(); }
#endif
  private:
    tw_stime _simTime;
  };
}
#endif

namespace xpdes {
#ifdef RTSS_LIBRARY_WITH_ROSS
  inline void initializePDES();
  inline void beginForwardEvent(void* twlp);
  // only required if no commit function is provided by ROSS
  inline void beginForwardEvent(void* twlp, Backstroke::SimTime simTime);
  //inline void endForwardEvent(twlp);
  inline void reverseEvent(void* twlp);
  inline void commitEvent(void* twlp);
  // only required if no commit function is provided by ROSS
  inline void commitEvent(tw_lp* twlp, Backstroke::SimTime now);
#endif
} // end of namespace xpdes

namespace rtss=xpdes;

#include <stack>
#include <queue>
#include <deque>
#include <map>
#include <list>
#include <string>
#ifdef BS_DEBUG_PRINT
#include <iostream>
#include <sstream>
#endif
#include <exception>
#include <inttypes.h>

// TODO: with C++11 the next line can be removed
#include <tr1/type_traits>

// TODO: with C++11 the next line can be removed
#include <cstdlib>

#include <cassert>

#define BS_ALLOC_OBJ
#define BS_ALLOC_ARRAY
#define BS_DEALLOC_OBJ
#define BS_DEALLOC_ARRAY

namespace Backstroke {
  
#include "backstroke/rtss-macros.h"
  
  class Exception : public std::exception {
  public:
    Exception(std::string text) {
#pragma reversible map forward=original
      this->text=text;
    }
    const char* what() const throw() {
      return text.c_str();
    }
    ~Exception() throw() {
    }
  private:
    std::string text;
  };
  
// template function for array operator delete[]
#pragma reversible map forward=original
template <typename ArrayElementType>
ArrayElementType* callArrayElementDestructors(ArrayElementType* arrayPointer) {
  std::size_t* rawMemory=reinterpret_cast<std::size_t*>(arrayPointer)-1;
  std::size_t arraySize=*rawMemory;
  if(arrayPointer != 0) {    
    ArrayElementType *p = arrayPointer + arraySize;
    while (p != arrayPointer) {
      (--p)->~ArrayElementType();
    }
  }
  return arrayPointer;
}

class RunTimeStateStore {
 private:
  enum BuiltInType {
    BITYPE_BOOL,
    BITYPE_CHAR,
    BITYPE_SHORT_INT,
    BITYPE_INT,
    BITYPE_LONG_INT,
    BITYPE_LONG_LONG_INT,
    BITYPE_USHORT_INT,
    BITYPE_UINT,
    BITYPE_ULONG_INT,
    BITYPE_ULONG_LONG_INT,
    BITYPE_FLOAT,
    BITYPE_DOUBLE,
    BITYPE_LONG_DOUBLE,
    BITYPE_PTR,
    BITYPE_REVERSE_FUNCTION_CALL_NO_ARGS,
    //BITYPE_REVERSE_FUNCTION_CALL_JANUS,
    BITYPE_NUM
  };
 public:
  typedef void* ptr;
  struct EventRecord {
    inline void rollbackRegisteredAllocations();
    inline void disposeRegisteredAllocations();
    inline void commitRegisteredDeallocations();
    inline void disposeRegisteredDeallocations();
    std::stack<BuiltInType> stack_bitype;
    std::queue<ptr> registeredHeapAllocation;
    std::queue<ptr> registeredHeapDeallocation;
    std::queue<ptr> registeredHeapArrayAllocation;
    std::queue<ptr> registeredHeapArrayDeallocation;
    inline void reset();
#ifdef RTSS_LIBRARY_WITH_ROSS
    SimTime simTime;
#endif
  };
  inline RunTimeStateStore();
  inline RunTimeStateStore(int n);
  inline ~RunTimeStateStore();
  inline void beginForwardEvent();
  inline void endForwardEvent();
  inline void reverseEvent();
  inline void commitEvent();
  inline void deallocate(EventRecord* commitEventRecord);
  inline ptr registerAllocation(ptr p);
  inline void registerArrayAllocation(void* rawMemory);
  inline void registerDeallocation(ptr p);
  inline void registerReverseFunctionCallNoArgs(ptr p);
#ifdef RTSS_LIBRARY_WITH_ROSS
  inline void setEventSimTime(Backstroke::SimTime simTime);
  inline void commitEventsLessThanSimTime(Backstroke::SimTime simTime);
#endif

  std::size_t numberOfUncommittedEvents();
  std::size_t size();
  std::size_t currentEventLength();

  template <typename T> 
  T*& avpushT(T*& lhs) {
    if(dataRecording()==true) {
      disableDataRecording();
      avpushptr((void**)(&lhs));
      enableDataRecording();
    }
    return lhs;
  }
  // is_integral<T>/is_float<T> do not give exected behavior in combination
  // with all the other checks, therefore functions are overloaded here
#define INTEGRAL_PUSH(type) type& avpushT(type& lhs) {\
    if(Backstroke::RunTimeStateStore::dataRecording()==true) {\
      Backstroke::RunTimeStateStore::disableDataRecording();\
      avpush(&lhs);\
      Backstroke::RunTimeStateStore::enableDataRecording();\
    }\
    return lhs;\
  }\

  INTEGRAL_PUSH(bool)
  INTEGRAL_PUSH(char)
  INTEGRAL_PUSH(short)
  INTEGRAL_PUSH(unsigned short)
  INTEGRAL_PUSH(int)
  INTEGRAL_PUSH(unsigned int)
  INTEGRAL_PUSH(long)
  INTEGRAL_PUSH(unsigned long)
  INTEGRAL_PUSH(long long)
  INTEGRAL_PUSH(unsigned long long)
  INTEGRAL_PUSH(float)
  INTEGRAL_PUSH(double)
  INTEGRAL_PUSH(long double)

  template <typename T> 
    T& avpushT(T& lhs) {
    if(dataRecording()==true) {
      disableDataRecording();
      if(std::tr1::is_class<T>::value||std::tr1::is_union<T>::value) {
        // fall through
      } else if(std::tr1::is_enum<T>::value) {
        // enum is guaranteed to have size of int
        avpush((int*)&lhs);
      } else if(std::tr1::is_member_pointer<T>::value) {
        avpushptr((void**)&lhs);
      } else if(std::tr1::is_member_object_pointer<T>::value) {
        avpushptr((void**)&lhs);
      } else if(std::tr1::is_member_function_pointer<T>::value) {
        avpushptr((void**)&lhs);
      } else {
        throw Backstroke::Exception("RTSS:avpushT: unknown type.");
      }
    } else {
      // dataRecording()==false
      return lhs;
    }
    enableDataRecording();
    return lhs;
  }

  template <typename T>
    T* registerAllocationT(T* mem) {
    // a rollback of an allocation performs a deallocation
    if(dataRecording()) {
      disableDataRecording();
      registerAllocation((void*)mem);
      enableDataRecording();
      return mem;
    } else {
      return mem;
    }
  }

#pragma reversible map forward=original
  template <typename T>
    T* allocateArrayT(std::size_t numberOfElements) {
    if(dataRecording()) {
      disableDataRecording();
      T* mem=reinterpret_cast<T*>(allocateArray(numberOfElements,sizeof(T)));
      enableDataRecording();
      return mem;
    } else {
      return new T[numberOfElements];
    }
  }

  template <typename C>
    // TODO: what if destructor is protected or private? It cannot be called.
    void destructorcall (C* mem) {
    mem->~C();
  }

#pragma reversible map forward=original
  template <typename T>
    void registerDeallocationT(T* mem) {
    if(dataRecording()) {
      if(std::tr1::is_class<T>::value||std::tr1::is_union<T>::value) {
        destructorcall(mem);
      }
      disableDataRecording();
      registerDeallocation((void*)mem);
      enableDataRecording();
    } else {
      delete mem;
    }
  }

#pragma reversible map forward=original
  template <typename T>
    void operatorDeleteT(T* mem, const std::nothrow_t& tag) {
    if(dataRecording()) {
      // no destructor handling neccessary in constrast to delete
      disableDataRecording();
      registerDeallocation((void*)mem); // TODO: ADD SECOND PARAMETER! registerDeallaction(mem,tag);
      enableDataRecording();
    } else {
      ::operator delete(mem,tag);
    }
  }

#pragma reversible map forward=original
  template <typename T>
    void operatorDeleteT(T* mem) {
    if(dataRecording()) {
      // no destructor handling neccessary in constrast to delete
      disableDataRecording();
      registerDeallocation((void*)mem);
      enableDataRecording();
    } else {
      ::operator delete(mem);
    }
  }

#pragma reversible map forward=original
  template <typename T>
    void registerArrayDeallocationT(T* deleteOperand) {
    if(dataRecording()) {
      if(deleteOperand) {
        if(std::tr1::is_class<T>::value) {
          // destructors can be BS instrumented (therefore mode remains on here)
          T* temp=Backstroke::callArrayElementDestructors(deleteOperand);
          disableDataRecording();
          registerArrayDeallocation((void*)temp);
          enableDataRecording();
        } else {
          disableDataRecording();
          registerArrayDeallocation((void*)deleteOperand);
          enableDataRecording();
        }
      }
    } else {
      // for correct original code this operator will match operator 'new[]'
      delete[] deleteOperand;
    }
  }

  // this template only exists because the pragma cannot be used inside the macro
#pragma reversible map forward=original
  template <typename T>
  void restore_assignment_inside_macro(std::pair<T*,T>& p) {
    *(p.first)=p.second;
  }
  // headers with private data members for all built-in types
  AVPUSH_RESTORE_DISPOSE_HEADER(bool,bool)
  AVPUSH_RESTORE_DISPOSE_HEADER(char,char)
  AVPUSH_RESTORE_DISPOSE_HEADER(short int,shortint)
  AVPUSH_RESTORE_DISPOSE_HEADER(int,int)
  AVPUSH_RESTORE_DISPOSE_HEADER(long int,longint)
  AVPUSH_RESTORE_DISPOSE_HEADER(long long int,longlongint)
  AVPUSH_RESTORE_DISPOSE_HEADER(unsigned short int,ushortint)
  AVPUSH_RESTORE_DISPOSE_HEADER(unsigned int,uint)
  AVPUSH_RESTORE_DISPOSE_HEADER(unsigned long int,ulongint)
  AVPUSH_RESTORE_DISPOSE_HEADER(unsigned long long int,ulonglongint)
  AVPUSH_RESTORE_DISPOSE_HEADER(float,float)
  AVPUSH_RESTORE_DISPOSE_HEADER(double,double)
  AVPUSH_RESTORE_DISPOSE_HEADER(long double,longdouble)

  // headers for ptr handling
 private:
  std::deque<std::pair<ptr*, ptr> > data_container_ptr;
  inline void restore_ptr();
  inline void dispose_ptr();
 public:
  //inline ptr assignptr(ptr* address, ptr value);
  inline ptr* avpush(ptr* address) { return avpushptr(address); }
  inline ptr* avpushptr(ptr* address);

 public:
  // must be called to initialize run-time-state storage (shared between different instances)
  static void init_stack_info();
  static void print_stack_info();

  inline bool is_stack_ptr(void *ptr) {
    return ((uintptr_t) ptr >= (uintptr_t) prog_stack_bottom)
      && ((uintptr_t) ptr <= (uintptr_t) prog_stack_local);
  }
  // clears all internal data structures
  inline void reset();

  inline void restore(BuiltInType bitype);
  inline void dispose(BuiltInType bitype);

  std::deque<EventRecord*> eventRecordDeque;
  std::size_t typeSize(BuiltInType biType);
  EventRecord* currentEventRecord;

  // STACK/HEAP detection
  static uintptr_t prog_stack_bottom;
  // can be set to function stack (default: same as stack_bottom)
  static uintptr_t prog_stack_local;
  static uintptr_t prog_stack_max;

  // alloc functions
  inline void* allocateArray(std::size_t arraySize, std::size_t ArrayElementTypeSize);

  // dealloc functions
  // also requires callArrayElementDestructors(ArrayElementType* arrayPointer) to be called
  inline void registerArrayDeallocation(void* rawMemoryPtr);
  static void deallocateArray(void* rawMemoryPtr);

  static bool enableDataRecording() { 
    bool tmp=_dataRecording;
#pragma reversible map forward=original
    _dataRecording=true;
    return tmp;
  }
  static bool disableDataRecording() { 
    bool tmp=_dataRecording;
#pragma reversible map forward=original
    _dataRecording=false; 
    return tmp;
  }
  static bool dataRecording() { 
    return _dataRecording; 
  }
  static bool setDataRecording(bool flag) {
    bool tmp=_dataRecording;
#pragma reversible map forward=original
    _dataRecording=flag;
    return tmp;
  }

 private:
  static bool _dataRecording;
  enum EventRecordAllocMode { ALLOCMODE_DEFAULT, ALLOCMODE_POOL };
  EventRecordAllocMode _eventRecordAllocMode;
  // for pre-allocated event records (using constructor RunTimeStateStore(n))
  inline void initEventRecordPool(int n);
  inline EventRecord* allocateEventRecord();
  inline void deallocateEventRecord(EventRecord* eventRecord);
  std::list<EventRecord*> eventRecordPoolAvailable;
};

#if 1
#define BS_DATA_PROTECT_BEGIN Backstroke::RunTimeStateStore::disableDataRecording()
#define BS_DATA_PROTECT_END Backstroke::RunTimeStateStore::enableDataRecording()
#else
#define BS_DATA_PROTECT_BEGIN
#define BS_DATA_PROTECT_END
#endif

#ifdef RTSS_LIBRARY_WITH_ROSS
#include "ross.h"
 class RunTimeLpStateStore {
   typedef std::map<void*, RunTimeStateStore*> LpToRTSSMapping;
 public:
   RunTimeStateStore* getLpStateStorage(void* lp);
   void setLpStateStorage(void* lp, RunTimeStateStore* ss);
   bool isSetLp(void* lp);
   RunTimeStateStore* getLpStateStorageAutoAllocated(void* lp);
   RunTimeStateStore* getLpStateStorageAutoAllocated(void* lp,std::size_t num);
 private:
   LpToRTSSMapping lp_ss_mapping;
 };
#endif

} // end of namespace Backstroke

#ifdef RTSS_LIBRARY_WITH_ROSS
inline Backstroke::SimTime::SimTime():_simTime(-1.0) {
}

inline Backstroke::SimTime::SimTime(tw_stime simTime):_simTime(simTime) {
}

inline bool Backstroke::SimTime::operator<(const SimTime& other) {
  return _simTime<other._simTime;
}
#endif

#pragma reversible map forward=original
inline void Backstroke::RunTimeStateStore::reset() {
  // clear event-record deqeue
  //std::cout<<"RESET-start: run time system: ER:"<<eventRecordDeque.size()<<" PTR-STACK:"<<stack_ptr.size()<<std::endl;
  while(!eventRecordDeque.empty()) {
    EventRecord* eventRecord=eventRecordDeque.front();
    eventRecord->reset();
    assert(eventRecord->stack_bitype.size()==0);
    delete eventRecord;
    eventRecordDeque.pop_front();
  } 
  assert(eventRecordDeque.size()==0 && data_container_ptr.size()==0);

  //std::cout<<"RESET-done : run time system: ER:"<<eventRecordDeque.size()<<" PTR-STACK:"<<stack_ptr.size()<<std::endl;
}

// create a clean object (for reuse)
#pragma reversible map forward=original
void Backstroke::RunTimeStateStore::EventRecord::reset() {
  assert(stack_bitype.size()==0);

  // only deallocations can remain, if the original program does alloc/dealloc within an event
  assert(registeredHeapAllocation.empty());
  while(!registeredHeapDeallocation.empty()) registeredHeapDeallocation.pop();
  assert(registeredHeapArrayAllocation.empty());
  while(!registeredHeapArrayDeallocation.empty()) registeredHeapArrayDeallocation.pop();
}


// this function is called when an event is reversed ("de-register")
#pragma reversible map forward=original
inline void Backstroke::RunTimeStateStore::EventRecord::rollbackRegisteredAllocations() {
  while(!registeredHeapAllocation.empty()) {
    ptr toDeallocPtr=registeredHeapAllocation.front();
    registeredHeapAllocation.pop();
    // we only need to call delete(x) because ~X() has already been called in the forward method
    ::operator delete(toDeallocPtr);
  }
  while(!registeredHeapArrayAllocation.empty()) {
    ptr toDeallocArrayRawPtr=registeredHeapArrayAllocation.front();
    registeredHeapArrayAllocation.pop();
    // we only need to call delete(x) because ~X() for all objects have already been called in the forward method
    Backstroke::RunTimeStateStore::deallocateArray(toDeallocArrayRawPtr);
  }
}

#pragma reversible map forward=original
void Backstroke::RunTimeStateStore::EventRecord::disposeRegisteredAllocations() {
   while(!registeredHeapAllocation.empty())
     registeredHeapAllocation.pop();
   while(!registeredHeapArrayAllocation.empty())
     registeredHeapArrayAllocation.pop();
}

#pragma reversible map forward=original
void Backstroke::RunTimeStateStore::EventRecord::disposeRegisteredDeallocations() {
   while(!registeredHeapDeallocation.empty())
     registeredHeapDeallocation.pop();
   while(!registeredHeapArrayDeallocation.empty())
     registeredHeapArrayDeallocation.pop();
}

// this function is called when an event is when an event is committed.
#pragma reversible map forward=original
void Backstroke::RunTimeStateStore::EventRecord::commitRegisteredDeallocations() {
  while(!registeredHeapDeallocation.empty()) {
    ptr toDeallocPtr=registeredHeapDeallocation.front();
    registeredHeapDeallocation.pop();
    // we only need to call delete(x) because ~X() has already been called in the forward method
    ::operator delete(toDeallocPtr);
  }
  while(!registeredHeapArrayDeallocation.empty()) {
    ptr toDeallocArrayRawPtr=registeredHeapArrayDeallocation.front();
    registeredHeapArrayDeallocation.pop();
    // we only need to call delete(x) because ~X() for all objects have already been called in the forward method
    Backstroke::RunTimeStateStore::deallocateArray(toDeallocArrayRawPtr);
  }
}

#pragma reversible map forward=original
void Backstroke::RunTimeStateStore::initEventRecordPool(int n) {
  for(int i=0;i<n;i++) {
    eventRecordPoolAvailable.push_front(new EventRecord());
  }
}

#pragma reversible map forward=original
Backstroke::RunTimeStateStore::EventRecord* Backstroke::RunTimeStateStore::allocateEventRecord() {
  switch(_eventRecordAllocMode) {
  case Backstroke::RunTimeStateStore::ALLOCMODE_DEFAULT:
    return new EventRecord();
  case ALLOCMODE_POOL: {
    if(eventRecordPoolAvailable.begin()!=eventRecordPoolAvailable.end()) {
      EventRecord* eventRecord=eventRecordPoolAvailable.front();
      assert(eventRecord);
      eventRecordPoolAvailable.pop_front();
      //std::cout<<"Reusing existing event:"<<eventRecord<<std::endl;
      return eventRecord;
    } else {
      // beyond pool size, extend pool
      EventRecord* eventRecord=new EventRecord();
      return eventRecord;
    }
    break;
  }
  default:
    throw Backstroke::Exception("unknown event record allocation mode.");
  }
}

#pragma reversible map forward=original
void Backstroke::RunTimeStateStore::deallocateEventRecord(EventRecord* eventRecord) {
  switch(_eventRecordAllocMode) {
  case ALLOCMODE_DEFAULT:
    delete eventRecord;
    break;
  case ALLOCMODE_POOL:
    // reinit element (prepare for reuse)
    eventRecord->reset();
        // add pointer to available-list
    eventRecordPoolAvailable.push_front(eventRecord);
    break;
  default:
    throw Backstroke::Exception("unknown event record allocation mode.");
  }
}

#pragma reversible map forward=original
Backstroke::RunTimeStateStore::RunTimeStateStore(int n):currentEventRecord(0) {
  _eventRecordAllocMode=ALLOCMODE_POOL;
  initEventRecordPool(n);
}

#pragma reversible map forward=original
Backstroke::RunTimeStateStore::RunTimeStateStore():currentEventRecord(0) {
  _eventRecordAllocMode=ALLOCMODE_DEFAULT;
}

#pragma reversible map forward=original
Backstroke::RunTimeStateStore::~RunTimeStateStore() {
  // clear all queues and stacks
  //this->reset();

  // handle modes for allocation of event records
  switch(_eventRecordAllocMode) {
  case Backstroke::RunTimeStateStore::ALLOCMODE_DEFAULT:
    break;
  case ALLOCMODE_POOL:
    while(eventRecordPoolAvailable.begin()!=eventRecordPoolAvailable.end()) {
      EventRecord* eventRecord=eventRecordPoolAvailable.front();
      eventRecordPoolAvailable.pop_front();
      eventRecord->reset();
      delete eventRecord;
    } 
    break;
  } // switch
}

inline std::size_t Backstroke::RunTimeStateStore::numberOfUncommittedEvents() {
  return eventRecordDeque.size();
}

inline std::size_t Backstroke::RunTimeStateStore::size() {
  return eventRecordDeque.size();
}

// allocates EventRecord
#pragma reversible map forward=original
inline void Backstroke::RunTimeStateStore::beginForwardEvent() {
  if(dataRecording()==true) {
    throw("RTSS:beginForwardEvent: data recorording enabled.");
  }
  currentEventRecord=allocateEventRecord();
  enableDataRecording();
}

#pragma reversible map forward=original
inline void Backstroke::RunTimeStateStore::endForwardEvent() {
  if(dataRecording()==false) {
    throw("RTSS:endForwardEvent: data recorording disabled.");
  }
  disableDataRecording();
  eventRecordDeque.push_back(currentEventRecord);
  currentEventRecord=0;
}

// deallocates EventRecord
#pragma reversible map forward=original
inline void Backstroke::RunTimeStateStore::reverseEvent() {
  if(dataRecording()==true) {
    throw("RTSS:reverseEvent: data recorording enabled.");
  }
  if(eventRecordDeque.empty()) {
    throw Backstroke::Exception("reverseEvent: no existing event");
  } else {

    EventRecord* restorationEventRecord=eventRecordDeque.back();
    eventRecordDeque.pop_back();
    assert(restorationEventRecord);
    // 1) DATA
    while(!restorationEventRecord->stack_bitype.empty()) {
      BuiltInType bitype=restorationEventRecord->stack_bitype.top();
      restorationEventRecord->stack_bitype.pop();
      restore(bitype);
    }
   
    // 2) ALLOCATIONS/DEALLOCATIONS
    restorationEventRecord->rollbackRegisteredAllocations();
    restorationEventRecord->disposeRegisteredDeallocations();
    
    deallocateEventRecord(restorationEventRecord);
  }
}

inline Backstroke::RunTimeStateStore::ptr Backstroke::RunTimeStateStore::registerAllocation(ptr p) {
  currentEventRecord->registeredHeapAllocation.push(p);
  return p;
}

inline void Backstroke::RunTimeStateStore::registerDeallocation(ptr p) {
  currentEventRecord->registeredHeapDeallocation.push(p);
}

#ifdef RTSS_LIBRARY_WITH_ROSS
#pragma reversible map forward=original
inline void Backstroke::RunTimeStateStore::setEventSimTime(Backstroke::SimTime simTime) {
  currentEventRecord->simTime=simTime;
}

// deallocates EventRecord
#pragma reversible map forward=original
inline void Backstroke::RunTimeStateStore::commitEventsLessThanSimTime(Backstroke::SimTime simTime) {
  while(eventRecordDeque.size()>0) {
#pragma reversible map forward=original
    EventRecord* commitEventRecord=eventRecordDeque.front();
    assert(commitEventRecord!=0);
    if(commitEventRecord->simTime<simTime) {
      commitEvent();
    } else {
      // is the commit queue sorted by time?
      break;
    }
  }
}
#endif

// pops event-record from event-record-queue and deallocates event-record
#pragma reversible map forward=original
inline void Backstroke::RunTimeStateStore::commitEvent() {
  if(dataRecording()==true) {
    throw("RTSS:commitEvent: data recorording enabled.");
  }
  if(eventRecordDeque.empty()) {
    throw Backstroke::Exception("commitEvent: no existing event");
  } else {
    EventRecord* commitEventRecord=eventRecordDeque.front();
    eventRecordDeque.pop_front();
    
    // 1) DATA
    // pop all stored data from data queues
    while(!commitEventRecord->stack_bitype.empty()) {
      BuiltInType bitype=commitEventRecord->stack_bitype.top();
      commitEventRecord->stack_bitype.pop();
      dispose(bitype);
    }
    
    // 2) ALLOCATIONS/DEALLOCATIONS
    commitEventRecord->disposeRegisteredAllocations();
    commitEventRecord->commitRegisteredDeallocations();
    
    deallocateEventRecord(commitEventRecord);
  }
}

#pragma reversible map forward=original
inline void Backstroke::RunTimeStateStore::dispose(BuiltInType bitype) {
  switch(bitype) {
    CASE_ENUM_DISPOSE(BOOL,bool);
    CASE_ENUM_DISPOSE(CHAR,char);
    CASE_ENUM_DISPOSE(SHORT_INT,shortint);
    CASE_ENUM_DISPOSE(INT,int);
    CASE_ENUM_DISPOSE(LONG_INT,longint);
    CASE_ENUM_DISPOSE(LONG_LONG_INT,longlongint);
    CASE_ENUM_DISPOSE(USHORT_INT,ushortint);
    CASE_ENUM_DISPOSE(UINT,uint);
    CASE_ENUM_DISPOSE(ULONG_INT,ulongint);
    CASE_ENUM_DISPOSE(ULONG_LONG_INT,ulonglongint);
    CASE_ENUM_DISPOSE(FLOAT,float);
    CASE_ENUM_DISPOSE(DOUBLE,double);
    CASE_ENUM_DISPOSE(LONG_DOUBLE,longdouble);
  case BITYPE_PTR: dispose_ptr();break;
  case BITYPE_REVERSE_FUNCTION_CALL_NO_ARGS: dispose_ptr();break;
  default:
    throw Backstroke::Exception("disposing unknown built-in type");
  }
}

#pragma reversible map forward=original
inline void Backstroke::RunTimeStateStore::restore(BuiltInType bitype) {
  switch(bitype) {
    CASE_ENUM_RESTORE(BOOL,bool);
    CASE_ENUM_RESTORE(CHAR,char);
    CASE_ENUM_RESTORE(SHORT_INT,shortint);
    CASE_ENUM_RESTORE(INT,int);
    CASE_ENUM_RESTORE(LONG_INT,longint);
    CASE_ENUM_RESTORE(LONG_LONG_INT,longlongint);
    CASE_ENUM_RESTORE(USHORT_INT,ushortint);
    CASE_ENUM_RESTORE(UINT,uint);
    CASE_ENUM_RESTORE(ULONG_INT,ulongint);
    CASE_ENUM_RESTORE(ULONG_LONG_INT,ulonglongint);
    CASE_ENUM_RESTORE(FLOAT,float);
    CASE_ENUM_RESTORE(DOUBLE,double);
    CASE_ENUM_RESTORE(LONG_DOUBLE,longdouble);
  case BITYPE_PTR: restore_ptr();break;
  case BITYPE_REVERSE_FUNCTION_CALL_NO_ARGS: dispose_ptr();break;
  default:
    throw Backstroke::Exception("restoring unknown built-in type.");
  }
}

AVPUSH_RESTORE_DISPOSE_IMPLEMENTATION(bool,bool,BITYPE_BOOL)
AVPUSH_RESTORE_DISPOSE_IMPLEMENTATION(char,char,BITYPE_CHAR)
AVPUSH_RESTORE_DISPOSE_IMPLEMENTATION(short int,shortint,BITYPE_SHORT_INT)
AVPUSH_RESTORE_DISPOSE_IMPLEMENTATION(int,int,BITYPE_INT)
AVPUSH_RESTORE_DISPOSE_IMPLEMENTATION(long int,longint,BITYPE_LONG_INT)
AVPUSH_RESTORE_DISPOSE_IMPLEMENTATION(long long int,longlongint,BITYPE_LONG_LONG_INT)
AVPUSH_RESTORE_DISPOSE_IMPLEMENTATION(unsigned short int,ushortint,BITYPE_USHORT_INT)
AVPUSH_RESTORE_DISPOSE_IMPLEMENTATION(unsigned int,uint,BITYPE_UINT)
AVPUSH_RESTORE_DISPOSE_IMPLEMENTATION(unsigned long int,ulongint,BITYPE_ULONG_INT)
AVPUSH_RESTORE_DISPOSE_IMPLEMENTATION(unsigned long long int,ulonglongint,BITYPE_ULONG_LONG_INT)
AVPUSH_RESTORE_DISPOSE_IMPLEMENTATION(float,float,BITYPE_FLOAT)
AVPUSH_RESTORE_DISPOSE_IMPLEMENTATION(double,double,BITYPE_DOUBLE)
AVPUSH_RESTORE_DISPOSE_IMPLEMENTATION(long double,longdouble,BITYPE_LONG_DOUBLE)

inline Backstroke::RunTimeStateStore::ptr* Backstroke::RunTimeStateStore::avpushptr(Backstroke::RunTimeStateStore::ptr* address) {
  if(!is_stack_ptr(address)) {
    currentEventRecord->stack_bitype.push(BITYPE_PTR);
    data_container_ptr.push_back(std::make_pair(address,*address));
  }
  return address;
}

#pragma reversible map forward=original
inline void Backstroke::RunTimeStateStore::restore_ptr() {
  std::pair<ptr*,ptr> p=data_container_ptr.back();
  data_container_ptr.pop_back();
  *(p.first)=p.second;
}

#pragma reversible map forward=original
inline void Backstroke::RunTimeStateStore::dispose_ptr() {
  data_container_ptr.pop_front();
}

#pragma reversible map forward=original
inline std::size_t Backstroke::RunTimeStateStore::typeSize(BuiltInType biType) {
  switch(biType) {
    CASE_ENUM_SIZEOF(BOOL,bool);
    CASE_ENUM_SIZEOF(CHAR,char);
    CASE_ENUM_SIZEOF(SHORT_INT,short int);
    CASE_ENUM_SIZEOF(INT,int);
    CASE_ENUM_SIZEOF(LONG_INT,long int);
    CASE_ENUM_SIZEOF(LONG_LONG_INT,long long int);
    CASE_ENUM_SIZEOF(USHORT_INT,unsigned short int);
    CASE_ENUM_SIZEOF(UINT,unsigned int);
    CASE_ENUM_SIZEOF(ULONG_INT,unsigned long int);
    CASE_ENUM_SIZEOF(ULONG_LONG_INT,unsigned long long int);
    CASE_ENUM_SIZEOF(FLOAT,float);
    CASE_ENUM_SIZEOF(DOUBLE,double);
    CASE_ENUM_SIZEOF(LONG_DOUBLE,long double);
    CASE_ENUM_SIZEOF(PTR,ptr);
    CASE_ENUM_SIZEOF(REVERSE_FUNCTION_CALL_NO_ARGS,void(*)());

  default:
    throw Backstroke::Exception("size of unsupported built-in type.");
  }
}

#pragma reversible map forward=original
inline std::size_t Backstroke::RunTimeStateStore::currentEventLength() {
  return currentEventRecord->stack_bitype.size();
}

#pragma reversible map forward=original
inline void* Backstroke::RunTimeStateStore::allocateArray(std::size_t arraySize, std::size_t arrayElementTypeSize) {
  // allocate one additional std::size_t for size
  std::size_t* rawMemory=static_cast<std::size_t*>(::operator new (static_cast<std::size_t>(arraySize*arrayElementTypeSize)+sizeof(std::size_t)));
  // store size
  *rawMemory=arraySize;
  // register raw Memory pointer for rollback
  registerArrayAllocation(rawMemory);
  // return array-pointer (excluding size field)
  void* arrayPointer=reinterpret_cast<void*>(rawMemory+1);
  return arrayPointer;
}

#pragma reversible map forward=original
inline void Backstroke::RunTimeStateStore::registerArrayAllocation(void* rawMemory) {
  // register array ptr 
  currentEventRecord->registeredHeapArrayAllocation.push(rawMemory);
}

#pragma reversible map forward=original
inline void Backstroke::RunTimeStateStore::registerArrayDeallocation(void* arrayPtr) {
  // convert(arrayPtr,rawPtr);
  if(arrayPtr) {
    std::size_t* rawMemory=(static_cast<std::size_t*>(arrayPtr))-1;
    currentEventRecord->registeredHeapArrayDeallocation.push(rawMemory);
  }
}

#pragma reversible map forward=original
inline void Backstroke::RunTimeStateStore::deallocateArray(void* rawMemoryPtr) {
  ::operator delete(rawMemoryPtr);
}

#ifdef RTSS_LIBRARY_WITH_ROSS
#pragma reversible map forward=original
inline Backstroke::RunTimeStateStore* Backstroke::RunTimeLpStateStore::getLpStateStorage(void* lp) {
  return lp_ss_mapping[lp];
}

#pragma reversible map forward=original
inline void Backstroke::RunTimeLpStateStore::setLpStateStorage(void* lp, RunTimeStateStore* ss) {
  lp_ss_mapping[lp]=ss;
}

#pragma reversible map forward=original
inline bool Backstroke::RunTimeLpStateStore::isSetLp(void* lp) {
  LpToRTSSMapping::iterator it=lp_ss_mapping.find(lp);
  return it!=lp_ss_mapping.end();
}

#pragma reversible map forward=original
inline Backstroke::RunTimeStateStore* Backstroke::RunTimeLpStateStore::getLpStateStorageAutoAllocated(void* lp) {
  return getLpStateStorageAutoAllocated(lp,0);
}

#pragma reversible map forward=original
inline Backstroke::RunTimeStateStore* Backstroke::RunTimeLpStateStore::getLpStateStorageAutoAllocated(void* lp,std::size_t num) {
  if(isSetLp(lp)) {
    return lp_ss_mapping[lp];
  } else {
    RunTimeStateStore* myrtss;
    if(num>0) {
      myrtss=new RunTimeStateStore(num);
    } else {
      myrtss=new RunTimeStateStore();
    }
    lp_ss_mapping[lp]=myrtss;
    return myrtss;
  }
}

#pragma reversible map forward=original
inline void xpdes::initializePDES() {
  Backstroke::RunTimeStateStore::init_stack_info();
  xpdes::rtlpss=new Backstroke::RunTimeLpStateStore();
}

#pragma reversible map forward=original
inline void xpdes::beginForwardEvent(void* twlp) {
  xpdes::internalRTSS=xpdes::rtlpss->getLpStateStorageAutoAllocated(twlp);
  xpdes::internalRTSS->beginForwardEvent();
}

// only required if no commit function is provided by ROSS
#pragma reversible map forward=original
inline void xpdes::beginForwardEvent(void* twlp, Backstroke::SimTime now) {
  xpdes::beginForwardEvent(twlp);
  xpdes::internalRTSS->setEventSimTime(now);
}

#pragma reversible map forward=original
inline void xpdes::reverseEvent(void* twlp) {
  xpdes::internalRTSS=xpdes::rtlpss->getLpStateStorageAutoAllocated(twlp);
  assert(xpdes::internalRTSS);
  xpdes::internalRTSS->reverseEvent();
}

#pragma reversible map forward=original
inline void xpdes::commitEvent(void* twlp) {
  xpdes::internalRTSS=xpdes::rtlpss->getLpStateStorageAutoAllocated(twlp);
  assert(xpdes::internalRTSS);
  xpdes::internalRTSS->commitEvent();
}

// only required if no commit function is provided by ROSS
#pragma reversible map forward=original
inline void xpdes::commitEvent(tw_lp* twlp, Backstroke::SimTime now) {
  xpdes::internalRTSS=xpdes::rtlpss->getLpStateStorageAutoAllocated(twlp);
  Backstroke::SimTime stime(twlp->pe->GVT);
  xpdes::internalRTSS->commitEventsLessThanSimTime(stime);
}


// end of ROSS specific xpdes interface
#endif

inline bool xpdes::enableDataRecording() {
  return Backstroke::RunTimeStateStore::enableDataRecording();
}

inline bool xpdes::disableDataRecording() {
  return Backstroke::RunTimeStateStore::disableDataRecording();
}

inline bool dataRecording() {
  return Backstroke::RunTimeStateStore::dataRecording();
}

inline bool xpdes::setDataRecording(bool flag) {
  return Backstroke::RunTimeStateStore::setDataRecording(flag);
}

///////////////////////////////////////////////////////////
// LEVEL 1                                               //
///////////////////////////////////////////////////////////

#pragma reversible map forward=original
inline void xpdes::initializeRTSS() {
  if(dataRecording()) {
    throw Backstroke::Exception("initalizeRTSS: data recording mode active.");
  }
  internalRTSS=new Backstroke::RunTimeStateStore();
  internalRTSS->reset();
  internalRTSS->init_stack_info();
}

#pragma reversible map forward=original
inline void xpdes::initializeRTSS(int n) {
  if(dataRecording()) {
    throw Backstroke::Exception("initalizeRTSS: data recording mode active.");
  }
  internalRTSS=new Backstroke::RunTimeStateStore(n);
  internalRTSS->init_stack_info();
}

#pragma reversible map forward=original
inline void xpdes::finalizeRTSS() {
  if(dataRecording()) {
    throw Backstroke::Exception("finalizeRTSS: data recording mode active.");
  }
  if(internalRTSS) {
    delete internalRTSS;
  }
}

inline void xpdes::beginForwardEvent() {
  internalRTSS->beginForwardEvent();
}

inline void xpdes::endForwardEvent() {
  internalRTSS->endForwardEvent();
}

inline void xpdes::reverseEvent() {
  internalRTSS->reverseEvent();
}

inline void xpdes::commitEvent() {
  internalRTSS->commitEvent();
}

inline bool xpdes::dataRecording() {
  return Backstroke::RunTimeStateStore::dataRecording();
}

///////////////////////////////////////////////////////////
// LEVEL 1                                               //
///////////////////////////////////////////////////////////
template <typename T> T& xpdes::avpushT(T& lhs) {
  return internalRTSS->avpushT(lhs);
}
  
template <typename T>  T*& xpdes::avpushT(T*& lhs) {
  return internalRTSS->avpushT(lhs);
}

template <typename T> T* xpdes::registerAllocationT(T* mem) {
  return internalRTSS->registerAllocationT(mem);
}

template <typename T> T* xpdes::registerOperatorNewT(T* mem) {
  return internalRTSS->registerAllocationT(mem);
}

template <typename T> void xpdes::registerDeallocationT(T* mem) {
  internalRTSS->registerDeallocationT(mem);
}

template <typename T> void xpdes::operatorDeleteT(T* mem) {
  internalRTSS->operatorDeleteT(mem);
}

#pragma reversible map forward=original
template <typename T> T* xpdes::allocateArrayT(size_t numberOfElements) {
  // check of mode is required (not performed in allocateArray)
  if(xpdes::dataRecording()) {
    T* a=(T*) internalRTSS->allocateArray(numberOfElements,sizeof(T));
    return a;
  } else {
    return new T[numberOfElements];
  }
}

#pragma reversible map forward=original
template <typename T> void xpdes::registerArrayDeallocationT(T* deleteOperand) {
  // check of mode is required (not performed in registerArrayDeallocation)
  if(xpdes::dataRecording()) {
    internalRTSS->registerArrayDeallocation(deleteOperand);
  } else {
    delete[] deleteOperand;
  }
}

#endif
