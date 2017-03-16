class A {
public:
  A():p(new int()) {
  }
  int* p;
  A(int x);
  ~A();
};


A::A(int x):p(new int()) {
  *p=x;
}

A::~A() {
  delete p;
}

int main() {
  A a;
  return 0;
}
