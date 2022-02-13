#include "Consumer_Handler.h"

#include "tao/x11/log.h"

// This function runs the Callback Quoter Consumer application.

int ACE_TMAIN (int argc, ACE_TCHAR* argv[])
{
  Consumer_Handler consumer;

  taox11_debug << "\n\t***Consumer***\n\n" << std::endl;

  if (consumer.init (argc, argv) == -1)
  {
    return -1;
  }

  return consumer.run ();
}
