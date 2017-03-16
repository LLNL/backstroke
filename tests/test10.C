#include <stdio.h>
#include <stdlib.h>

template<typename T>
class List {
public:
  List(T val);
  T getVal();
  List* getNext();
  void setNext(List*);
private:
  List* _next;
  T _val;
};

template<typename T>
List<T>::List(T val) : _next(0), _val(val) {}

template<typename T>
T List<T>::getVal() {
  return _val;
}

template<typename T>
List<T>* List<T>::getNext() {
  return _next;
}

template<typename T>
void List<T>::setNext(List<T>* next) {
  _next=next;
}
// --------------------------------------

List<int> *create_list(int n) {
  List<int> *head=0;
  List<int> *newElement;

  while (n>=0) {
    int r=rand();
    newElement = new List<int>(r);
    newElement->setNext(head);
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
List<int>* insert_iter(List<int>* head, int k) {
  List<int>* cur;
  List<int>* tail;
  cur = head;
  while (cur->getNext() != 0 && (cur->getNext()->getVal() < k)) {
    cur = cur->getNext();
  }
  tail = cur->getNext();

  List<int> *elem = new List<int>(k);
  elem->setNext(tail);
  cur->setNext(elem);

  return head;
}

// deletes all elements of a list
// PavluSchordanKrall10: new testcase
List<int>* delall_iter(List<int> *head) {
  List<int> *t;

  while (head != 0) {
    t = head->getNext();
    delete head;
    head = t;
  }
  return head;
}

List<int>* del_first(List<int> *head) {
  List<int> *t;
  if (head != 0) {
    t = head->getNext();
    delete head;
    head = t;
  }
  return head;
}

class State {
public:
  State():iter(0),list(0),numOps(50),maxlen(250000),containerSize(50000){}
  int iter;
  List<int>* list;
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

