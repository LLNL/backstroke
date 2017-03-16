// STL header <string>
namespace TEST {
typedef long int __jmp_buf[8];

typedef struct
{
  struct
  {
    __jmp_buf __cancel_jmp_buf;
    int __mask_was_saved;
  } __cancel_jmp_buf[1];
  void *__pad[4];
} __pthread_unwind_buf_t __attribute__ ((__aligned__));
} // end of namespace TEST

int main() {
  TEST::__pthread_unwind_buf_t buffer;
  return 0;
}
