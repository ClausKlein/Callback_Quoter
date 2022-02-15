#include "Consumer_Handler.h"

#include "tao/x11/log.h"

// This function runs the Callback Quoter Consumer application.

int ACE_TMAIN (int argc, ACE_TCHAR* argv[])
{
  // by default show all messages logged through global logger
  X11_LOGGER::priority_mask (x11_logger::X11_LogMask::LP_ALL);

  Consumer_Handler consumer;

  taox11_debug << "\n\t***Consumer***\n\n" << std::endl;

  if (consumer.init (argc, argv) == -1)
  {
    return -1;
  }

  return consumer.run ();
}
