#include <new>
int main() {
  int *ip1 =(int*)::operator new(sizeof(int));
  int *ip2;
  ip2=static_cast<int*>(::operator new(sizeof(int)));
  ::operator delete(ip2);
  ::operator delete(ip1);
  return 0;
}
