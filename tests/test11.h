#ifndef TEST11_H
#define TEST11_H

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

#endif
