#ifdef PRINT_RESULT
#include <cstdio>
#endif

#include <cassert>

class MyUserType {
public:
  MyUserType() {}
  ~MyUserType() {}
  int x;
};

class State {
public:
  State():count(0){}
  int count;
};

void event(State* s) {
  MyUserType* a=new MyUserType[10];

  const int n=10;
  for(int i=0;i<n;i++) {
    a[i].x=i+1;
  }

  int checksum=0;
  for(int i=0;i<n;i++) {
    checksum+=a[i].x;
  }
  delete[] a;
  const int expectedchecksum=n*(n+1)/2;
#ifdef PRINT_RESULT
printf("n=%d check-sum=%d expected-check-sum=%d\n",n,checksum,expectedchecksum);
#endif
  assert(checksum==expectedchecksum);
}

