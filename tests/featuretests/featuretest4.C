// multi-dimensional arrays
typedef long int DimArray1[2];
typedef long int DimArray2[2][3];
typedef long int DimArray3[2][3][4];

struct S {
  int a11[2];
  int a12[2][3];
  int a13[2][3][4];
  DimArray1 a21;
  DimArray2 a22;
  DimArray3 a23;
} structArray[100];


typedef double Matrix[3][3];
struct Precalculated
{
  Matrix a1[190];
  Matrix a2[190];
};

int main() {
  structArray[4].a11[1]=-10;
  structArray[4].a12[1][0]=-10;
  struct Precalculated s;
  s.a1[3][1][1]=1.0;
  double x;
  x=s.a1[3][1][1];
  return (int)x!=1.0;
}
