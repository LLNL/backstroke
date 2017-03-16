
class State {
public:
  State():count(0){}
  int count;
  void update1();
  void update2();
};

#pragma reversible map forward = original
void State::update1() {
  count++;
}

void State::update2() {
  count++;
}

#pragma other
#pragma reversible map forward = original
void f1(State* s) {
  s->count++;
}

void f2(State* s) {
  s->count++;
}

void event(State* s) {
  #pragma other
  #pragma reversible map forward = original
  #pragma other
  s->count++;
  #pragma reversible map forward = original
  #pragma other
  {
    s->count++;
    s->count++;
  }
  s->count++;
  #pragma other
  #pragma reversible map forward = original
  #pragma other
  {
  // test empty block followed by statement
  }
  s->count--;
}
