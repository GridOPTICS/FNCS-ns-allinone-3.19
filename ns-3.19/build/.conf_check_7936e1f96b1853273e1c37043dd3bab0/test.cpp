
#include<zmq.h>

int main()
{
  void *context = zmq_ctx_new();
  (void)zmq_term(context);
  return 0;
}
