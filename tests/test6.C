#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

//typedef int data;

struct data {
  short x;
  int y;
};

data* allocate_array(int n) {
  //printf("Allocating array\n");
  data* array=new data[n];

  while(n>0) {
    array[n-1].x=0;
    array[n-1].y=0;
    n--;
  }
  return array;
}

void delete_array(data *array) {
  //printf("Deleting array begin: %p\n",array);
  delete[] array;
  //printf("Deleting array finished.\n");
}

// array operation:
// array: pointer to start of array
// n: size of array
// k: number of operations
void operate_on_array(data* array, int n, int k) {
#if 0
  for(int i=0;i<k;i++) {
    int r1=rand()%n;
    int r2=rand()%n;
    int tmp=array[r1]+1;
    array[r1]=array[r2]+1;
    array[r2]=tmp;
  }
#else
  for(int i=0;i<k;i++) {
    int r=rand()%n;
    assert(r<n);
    assert(r>=0);
    array[r].x++;
    array[r].y++;
  }
#endif
}

class State {
public:
  State():iter(0),container(0),numOps(50),containerSize(1000000){}
  int iter;
  data* container;
  int numOps;
  int containerSize;
  // deletes all allocated data structures (for clean up in evaluations)
  void cleanup();
};

void State::cleanup() {
  //assert(container); // 0 is allowed
  delete[] container;
}

void event2(State* s) {
  //printf("event: s:%p iter:%d container:%p\n",s,s->iter,s->container);
  int n=s->containerSize;
  if(s->container==0) {
    s->container=allocate_array(n);
  } 
  if(s->iter<s->containerSize) {
    //printf("%d:%d\n",s->iter,s->containerSize);
    operate_on_array(s->container, n, s->numOps);
    s->iter++;
  }
  if(s->iter>=s->containerSize) {
    assert(s->iter==s->containerSize);
    delete_array(s->container);
    s->container=0;
    s->iter=0;
  }
#ifdef MYPRINT
  if(s->container) {
    s->container->print();
    //printf(" iter:%d max:%d\n",s->iter,s->containerSize);
  }
#endif
}

void event(State* s) {
  event2(s);
  return;
  /*
  int n=s->containerSize;
  assert(n>=0);
  if(s->container==0) {
    s->container=allocate_array(n);
  } else {
    operate_on_array(s->container, n, s->numOps);
  }
  */
}

#ifdef USE_MAIN
int main(int argc, char **argv) {
  State* s=new State();
  s->containerSize=10000;
  s->numOps=50;
  s->containerSize=100;
  for(int i=0;i<100000;i++) {
    event(s);
  }
  return 0;
}
#endif

