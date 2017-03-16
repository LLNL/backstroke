#include <stdio.h>
#include <stdlib.h>

class List {
public:
  List(int _d);
#ifdef MYPRINT
  void print() {
    printf("%d", d);
    if (n) {
      printf(", ");
      n->print();
    }
    else {
      printf("\n");
    }
  }
#endif
  List *n;
  int d;
};

List::List(int _d) : n(0), d(_d) {}

// --------------------------------------

List *create_list(int n) {
  List *head=0;
  List *newElement;

  while (n>=0) {
    int r=rand();
    newElement = new List(r);
    newElement->n = head;
    head = newElement;
    n--;
  }

  return head;
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
List* insert_iter(List* head, int k) {
  List* cur;
  List* tail;
  cur = head;
  while (cur->n != 0 && (cur->n->d < k)) {
    cur = cur->n;
  }
  tail = cur->n;

  List *elem = new List(k);
  elem->n = tail;
  cur->n  = elem;

  return head;
}

// deletes all elements of a list
// PavluSchordanKrall10: new testcase
List* delall_iter(List *head) {
  List *t;

  while (head != 0) {
    t = head->n;
    delete head;
    head = t;
  }
  return head;
}

List* del_first(List *head) {
  List *t;
  if (head != 0) {
    t = head->n;
    delete head;
    head = t;
  }
  return head;
}

class State {
public:
  State():iter(0),list(0),numOps(50),maxlen(250000),containerSize(50000){}
  int iter;
  List* list;
  int numOps;
  int maxlen;
  int containerSize;

  void cleanup() {
    delall_iter(list);
  }
};

void event(State* s) {
  int size=s->containerSize;
  if(s->list==0) {
    s->list=create_list(size);
  } else {
    int n=s->numOps;
    for(int i=0;i<n;++i) {
      int r=rand();
      s->list=insert_iter(s->list, r);
      s->list=del_first(s->list);
    }
  }
}

void event2(State* s) {
  int n=s->containerSize;
  if(s->iter==0||s->list==0) {
    s->list=create_list(n);
  } else if(s->iter<s->maxlen) {
    int n=s->numOps;
    for(int i=0;i<n;++i) {
      int r=rand();
      s->list=insert_iter(s->list, r);
      s->list=del_first(s->list);
    }
  } else {
    s->list=delall_iter(s->list);
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
  s->maxlen=100;
  for(int i=0;i<100000;i++) {
    event(s);
  }
  return 0;
}
#endif

