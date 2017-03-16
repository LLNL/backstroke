struct State {
  int x;
};

void event(State* state) {           
  if(state->x > 20) {
    state->x = 0;
  } else {
    state->x++;
  }
}
   
