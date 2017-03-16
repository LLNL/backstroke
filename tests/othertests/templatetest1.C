// template specialization test
int* x;

class X {
  int a;
  void f() {
    a=1;
#pragma reversible map forward=original
    a=2;
  }
};

template<typename T>
void swap(T a,T b) {
  T t=a;
  b=a;
  a=t;
#pragma reversible map forward=original
  t=t;
}

namespace std __attribute__ ((__visibility__ ("default")))
{
  template<bool _BoolType>
    struct __iter_swap
    {
      template<typename _ForwardIterator1, typename _ForwardIterator2>
        static void
        iter_swap(_ForwardIterator1 __a, _ForwardIterator2 __b)
        {
          typedef int
            _ValueType1;
          _ValueType1 __tmp = (*__a);
          *__a = (*__b);
          *__b = (__tmp);
#pragma reversible map forward=original
          *__b = (__tmp);
 }
    };
  template<>
    struct __iter_swap<true>
    {
      template<typename _ForwardIterator1, typename _ForwardIterator2>
        static void
        iter_swap(_ForwardIterator1 __a, _ForwardIterator2 __b)
        {
          swap(*__a, *__b);
          *x=5;
#pragma reversible map forward=original
          *x=5;
        }
    };
}

int main() {return 0;}
