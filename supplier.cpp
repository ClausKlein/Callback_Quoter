#include "Supplier_i.h"

#include "tao/x11/log.h"

// This function runs the Callback Quoter Supplier daemon.

int ACE_TMAIN (int argc, ACE_TCHAR* argv[])
{
  Supplier supplier;

  taox11_debug << "\n\tMarket Status Supplier Daemon\n\n" << std::endl;

  if (supplier.init (argc, argv) == -1)
  {
    return -1;
  }

  return supplier.run ();
}
