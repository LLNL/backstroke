#include <iostream>
#include <cstddef>
#include <cassert>

typedef void (*StaticWriteIntFuncFp)(void*,unsigned short, unsigned int);
typedef unsigned int (*StaticReadIntFuncFp)(void*,unsigned short);

struct S {
    // will usually occupy 2 bytes:
    // 3 bits: value of b1
    // 5 bits: unused
    // 6 bits: value of b2
    // 2 bits: value of b3
    unsigned int b1 : 3;
    unsigned int :0; // start a new byte
    unsigned int b2 : 9;
    unsigned int b3 : 2;
  S& operator=(S& other) {
    uintWriteBitField(this,0,(unsigned char)other.b1);
    uintWriteBitField(this,1,(unsigned int)other.b2);
    uintWriteBitField(this,2,(unsigned char)other.b3);
    return *this;
  }
  static void uintWriteBitField(void* myType, unsigned short nr, unsigned int val) {
    switch(nr) {
    case 0: ((S*)(myType))->b1=val;break;
    case 1: ((S*)(myType))->b2=val;break;
    case 2: ((S*)(myType))->b3=val;break;
    default:;
    }
  }
  static unsigned int uintReadBitField(void* myType, unsigned short nr) {
    switch(nr) {
    case 0: return ((S*)myType)->b1;
    case 1: return ((S*)myType)->b2;
    case 2: return ((S*)myType)->b3;
    default:;
    }
  }

};

using namespace std;

struct storage {
  StaticWriteIntFuncFp staticFpW;
  void* structFp;
  short memberNr;
  int value;
};

#include <queue>    
std::queue<storage> storageQueue;

unsigned int storeAndAssign(StaticReadIntFuncFp fpR,StaticWriteIntFuncFp fpW, void* s, short nr, unsigned int val) {
  storage st;
  st.staticFpW=fpW;
  st.structFp=s;
  st.memberNr=nr;
  st.value=(*fpR)(st.structFp,st.memberNr);
  storageQueue.push(st);
  (*fpW)(st.structFp,st.memberNr,val);
  return val;
}

void restore() {
  storage st=storageQueue.front();
  (*st.staticFpW)(st.structFp,st.memberNr,st.value);
  storageQueue.pop();
}

int main()
{
  S s = {1,2,3};
  s.b1=s.b1=s.b1;

  //StaticWriteIntFuncFp writeWrapFp=S::writeBitField_uint;
  //s.writeBitField_uint(&s,0,s.b1+1);

  // store/restore test
  // store(lhs-type-fp,lhs-ptr(struct),lhs-nr,rhs_exp)
  // s.b1=s.b1+1;
  storeAndAssign(S::uintReadBitField,S::uintWriteBitField,&(s),0,s.b1+5); // destroy b1-value
  assert(s.b1==6);
  restore();

  assert(s.b1==1);
  assert(s.b2==2);
  assert(s.b3==3);

  return 0;
}
