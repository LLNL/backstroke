struct S1 {
  // no default assignment operator
  S1(int a):y(a){}
  int x;
  const int y;
};

struct S2 {
  int x;
  int y;
};

int main() {
  S1 s1(1),s12(2);
  // s11=s12 not allowed
  S2 s21, s22;
  s21=s22;
  return 0;
}
