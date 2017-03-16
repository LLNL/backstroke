#include <stdio.h>
#include <assert.h>

class State {
public:
  State():charSequencePointer((char*)&charSequence) {
    // avoid reset to be called in constructor
  }
  void reset() {
    charSequencePointer=(char*)&charSequence;
  }
  void func() {
    int step=3;
    for(int i=0;i<256-step;i+=step) {
      charSequencePointer += step;
      assert(charSequencePointer>=charSequence);
      assert(charSequencePointer<(charSequence+256));
    }
    reset();
  }
private:
  char charSequence[256];
  char *charSequencePointer;
};

void event(State* state) {
  state->func();
}

#if 0
int main() {
  State s;
  event(&s);
  return 0;
}
#endif
