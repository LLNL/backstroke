// constructor initializer test without and with templates
class A {
public:
  A():a(1),b(b=2),c(new A()) {
  }
  A(int x);
  ~A() {
    delete c;
  }
  int a;
  int b;
  A* c;
};
A::A(int x):a(x),b(b=x),c(new A()) {}

template <class T>
class B {
public:
  B():a(1),b(b=2),c(new T()) {
  }
  B(int x);
  ~B() {
    delete c;
  }
  int a;
  int b;
  T* c;
};
template <class T>
B<T>::B(int x):a(x),b(b=x),c(new T()) {}

int main() { 
  A a1;
  A a2(1);
  B<int> b1;
  B<int> b2;
  return 0; 
}
