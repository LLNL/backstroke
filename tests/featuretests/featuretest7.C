struct S1 {
 // three-bit unsigned field,
 // allowed values are 0...7
 unsigned int b : 3;
};

struct S2 {
    // will usually occupy 2 bytes:
    // 3 bits: value of b1
    // 2 bits: unused
    // 6 bits: value of b2
    // 2 bits: value of b3
    // 3 bits: unused
    unsigned char b1 : 3, : 2, b2 : 6, b3 : 2;
};

struct S3 {
    // will usually occupy 2 bytes:
    // 3 bits: value of b1
    // 5 bits: unused
    // 6 bits: value of b2
    // 2 bits: value of b3
    unsigned char b1 : 3;
    unsigned char :0; // start a new byte
    unsigned char b2 : 6;
    unsigned char b3 : 2;
  S3& operator=(S3& other) {
    this->b1=other.b1;
    this->b2=other.b2;
    this->b3=other.b3;
    return *this;
  }
};

int main()
{
  int error=0;
  S1 s = {6};
  s.b=s.b+1;
  ++s.b; // unsigned overflow (guaranteed wrap-around)
  if(s.b!=0)
    error++;
  S2 s21,s22;
  s21=s22;
  return error;
}
