#include "Supplier_i.h"

#include "tao/x11/log.h"

// This function runs the Callback Quoter Supplier daemon.

int ACE_TMAIN (int argc, ACE_TCHAR* argv[])
{
  // by default show all messages logged through global logger
  X11_LOGGER::priority_mask (x11_logger::X11_LogMask::LP_ALL);

  Supplier supplier;

  taox11_debug << "\n\tMarket Status Supplier Daemon\n\n" << std::endl;

  if (supplier.init (argc, argv) == -1)
  {
    return -1;
  }

  return supplier.run ();
}
