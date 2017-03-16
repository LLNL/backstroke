int* create_iter(int n) {
  int* array=new int[n];
  while(n>0) {
    array[n-1]=n;
    n--;
  }
  return array;
}

// inserts an element into an ascending
// ordered list
// SagivRepsWilhelm98: insert
// changed second parameter:
//   was: pointer to existing elem
//   now: key used to create new elem
// changed return type:
//   was: void
//   now: List* to head of list
int* insert_iter(int* array, int n, int k) {
  for(int i=0;i<n;i++)
    if(array[i] >= k) {
      for(int j=n-1;j>=i;j--) {
        array[j]=array[j-1];
      }
      array[i]=k;
      break;
  }
  return array;
}

// deletes all elements of a list
// PavluSchordanKrall10: new testcase
void delall_iter(int *array) {
  delete[] array;
}

class State {
public:
  State():iter(0),list(0),maxlen(100){}
  int iter;
  int* list;
  int maxlen;
};

void event(State* s) {
  int n=50;
  if(s->iter==0||s->list==0) {
    s->list=create_iter(n);
  } else if(s->iter<s->maxlen) {
    insert_iter(s->list, n, s->iter);
  } else {
    delall_iter(s->list);
    s->list=0;
    s->iter=0;
  }
#ifdef MYPRINT
  if(s->list) {
    s->list->print();
    printf(" iter:%d max:%d\n",s->iter,s->maxlen);
  }
#endif
  s->iter++;
}

#if 0
int main(int argc, char **argv) {
  State* s=new State();
  s->maxlen=10000;
  for(int i=0;i<100000;i++) {
    event(s);
  }
  return 0;
}
#endif

