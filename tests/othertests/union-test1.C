class A {
public:
  union {
    int x;
  };
  int m() {
    x=1;
    return x;
  }
};

int main() {
  A a;
  a.m();
}
