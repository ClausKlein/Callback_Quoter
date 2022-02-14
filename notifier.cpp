#include "Notifier_Input_Handler.h"

#include "tao/x11/log.h"

// This is the main driver program for the Callback Quoter Notifier.

int ACE_TMAIN (int argc, ACE_TCHAR* argv[])
{
  Notifier_Input_Handler notifier;

  taox11_debug << "\n\tNotifier\n\n" << std::endl;

  try
  {
    int rc = notifier.init (argc, argv);
    if (rc == -1)
    {
      return 1;
    }

    notifier.run ();
  }
  catch (const CORBA::SystemException& sysex)
  {
    taox11_error << "System Exception: " << sysex << std::endl;
    return -1;
  }
  catch (const CORBA::UserException& userex)
  {
    taox11_error << "User Exception: " << userex << std::endl;
    return -1;
  }

  return 0;
}
