#include <iostream>

class A {
public:
  A():x(0),y(1){}
  A(const A& x);
  A(int x);
  A& operator=(const A& x);
protected:
  int x;
  int y;
};

class A2 {
  A2();
  A2(const A2& x) {}
  A2(int x) {}
  A2& operator=(const A2& x) {}
  int x;
  int y;
};

class B {
  //B();
  //  B(const A& x);
  B(int x);
  //B& operator=(const A& x);
  int x;
  int y;
};

void f(int& x) {
  std::cout<<"Data:"<<x<<std::endl;
};

class D : public A {
public:
  D() : A(),z((f(this->z),20)){
    std::cout<<"Dataconstr:"<<z<<std::endl;
  }
  int z;
};

template<typename T>
class C {
public:
  C() {}
  C(const C& x) {}
  C(int x):_x(x) {}
  C& operator=(const C& x) {}
  int _x;
  int y;
};


int main() {
  D d;
}
