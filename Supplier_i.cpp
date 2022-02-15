//=============================================================================
/**
 *  @file    Supplier_i.cpp
 *
 *  Implementation of the Supplier class.
 *
 *  @author Kirthika Parameswaran <kirthika@cs.wustl.edu>
 */
//=============================================================================

#include "Supplier_i.h"

#include "Naming_Client.h"

#include "ace/Get_Opt.h"

#include <fstream>

// Constructor.
Supplier::Supplier ()
  : supplier_timer_handler_ (nullptr)
  , argc_ (0)
  , argv_ (nullptr)
  , ior_ ()
  , use_naming_service_ (1)
  , notifier_ ()
  , f_ptr_ (nullptr)
  // UNUSED! , loop_count_ (10)
  , period_value_ (1)
{
  // No-op.
}

Supplier::~Supplier ()
{
  // Close the stream.
  ACE_OS::fclose (f_ptr_);

  taox11_debug << "Market Status Supplier Daemon exiting!" << std::endl;
}

// Reads the Server factory IOR from a file.
int Supplier::read_ior (const std::string& filename)
{
  // Open the file for reading.
  std::ifstream fis (filename);
  if (!fis)
  {
    taox11_error << "ERROR: failed to open file: " << filename << std ::endl;
    return -1;
  }

  std::getline (fis, this->ior_);
  fis.close ();

  return 0;
}

// Parses the command line arguments and returns an error status.
int Supplier::parse_args ()
{
  ACE_Get_Opt get_opts (argc_, argv_, ACE_TEXT ("dn:f:i:xk:xs"));

  int c;
  int result;

  while ((c = get_opts ()) != -1)
  {
    switch (c)
    {
      case 'd': // Debug flag
        // TODO TAO_debug_level++; //****
        break;

      case 'n': // Period_value: time between two successive stockfeeds.
        this->period_value_ = ACE_OS::atoi (get_opts.opt_arg ());
        break;

      case 'i': // Stock market information is got from a file.
        result = this->read_file (get_opts.opt_arg ());
        if (result < 0)
        {
          ACE_ERROR_RETURN ((LM_ERROR, "Unable to read stock information from %s : %p\n", get_opts.opt_arg ()), -1);
        }
        break;

      case 'k': // IOR provide on command line
        this->ior_ = get_opts.opt_arg ();
        break;

      case 'f': // Read the IOR from the file.
        result = this->read_ior (get_opts.opt_arg ());
        if (result < 0)
        {
          ACE_ERROR_RETURN ((LM_ERROR, "Unable to read ior from %s : %p\n", get_opts.opt_arg ()), -1);
        }
        break;

      case 's': // Don't use the naming service
        this->use_naming_service_ = 0;
        break;

      case '?':
      default:
        ACE_ERROR_RETURN ((LM_ERROR,
                           "usage:  %s"
                           " [-d]"
                           " [-n period]"
                           " [-f ior-file]"
                           " [-i input_filename]"
                           " [-k ior]"
                           " [-x]"
                           " [-s]"
                           "\n",
                           this->argv_[0]),
                          -1);
    }
  }

  // Indicates successful parsing of command line.
  return 0;
}

// Give the stock status information to the Notifier.
int Supplier::send_market_status (const char* stock_name, int32_t value)
{
  try
  {
    // Make the RMI.
    this->notifier_->market_status (stock_name, value);
  }
  catch (const CORBA::SystemException& sysex)
  {
    taox11_error << "System Exception : Supplier::send_market_status(): " << sysex << std::endl;
    return -1;
  }
  catch (const CORBA::UserException& userex)
  {
    taox11_error << "User Exception : Supplier::send_market_status(): " << userex << std::endl;
    return -1;
  }
  return 0;
}

// Execute client example code.
int Supplier::run () const
{
  long timer_id = 0;

  taox11_debug << "Market Status Supplier Daemon is running..." << std::endl;

  // This sets the period for the stock-feed.
  ACE_Time_Value period (this->period_value_);

  // "Your time starts now!" ;) the timer is scheduled to begin work.
  timer_id = reactor_used ()->schedule_timer (supplier_timer_handler_, "Periodic stockfeed", period, period);
  if (timer_id == -1)
  {
    ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "schedule_timer"), -1);
  }

  // The reactor starts executing in a loop.
  return reactor_used ()->run_reactor_event_loop ();
}

int Supplier::via_naming_service ()
{

#if 1
  // Get reference to initial naming context.
  IDL::traits<CosNaming::NamingContext>::ref_type inc = resolve_init<CosNaming::NamingContext> (this->orb_, "NameService");

  try
  {
    // Look for Notifier in the Naming Service.
    CosNaming::Name n (1);
    n[0].id ("Notifier");

    IDL::traits<CORBA::Object>::ref_type notifier_obj = resolve_name<Notifier> (inc, n);
    if (notifier_obj == nullptr)
    {
      taox11_error << "ERROR : resolved Notifier seems nill" << std::endl;
      return -1;
    }

    // The CORBA::Object object is downcast to Notifier using the <narrow> method.
    this->notifier_ = IDL::traits<Notifier>::narrow (notifier_obj);
  }
  catch (const CosNaming::NamingContext::NotFound& ex)
  {
    taox11_error << "No Notifier in Naming Service" << ex << std::endl;
    return -1;
  }
#else
  try
  {
    // XXX Initialization of the naming service.
    if (this->naming_services_client_.init (orb_) != 0)
      ACE_ERROR_RETURN ((LM_ERROR,
                         " (%P|%t) Unable to initialize "
                         "the TAO_Naming_Client.\n"),
                        -1);
    CosNaming::Name notifier_ref_name (1);
    notifier_ref_name.length (1);
    notifier_ref_name[0].id = "Notifier";

    IDL::traits<CORBA::Object>::ref_type notifier_obj = this->naming_services_client_->resolve (notifier_ref_name);

    // The CORBA::Object object is downcast to Notifier using the <_narrow> method.
    this->notifier_ = IDL::traits<Notifier>::narrow (notifier_obj);
  }
  catch (const CORBA::SystemException& sysex)
  {
    taox11_error << "Exception : Supplier::via_naming_service()" << sysex << std::endl;
    return -1;
  }
  catch (const CORBA::UserException& userex)
  {
    taox11_error << "Exception : Supplier::via_naming_service()" << userex << std::endl;
    return -1;
  }
#endif

  return 0;
}

// Init function.
int Supplier::init (int argc, char** argv)
{
  this->argc_ = argc;
  this->argv_ = argv;

  try
  {
    // Retrieve the ORB.
    this->orb_ = CORBA::ORB_init (this->argc_, this->argv_);

    // Parse command line and verify parameters.
    if (this->parse_args () == -1)
    {
      return -1;
    }

    // Create the Timer_Handler.
    ACE_NEW_RETURN (supplier_timer_handler_, Supplier_Timer_Handler (this, reactor_used (), this->f_ptr_), -1);

    if (this->use_naming_service_)
    {
      return via_naming_service ();
    }

    if (this->ior_.empty ())
    {
      ACE_ERROR_RETURN ((LM_ERROR, "%s: no ior specified\n", this->argv_[0]), -1);
    }
    IDL::traits<CORBA::Object>::ref_type notifier_object = this->orb_->string_to_object (this->ior_);

    if (notifier_object == nullptr)
    {
      ACE_ERROR_RETURN ((LM_ERROR, "invalid ior <%s>\n", this->ior_.c_str ()), -1);
    }

    // The downcasting from CORBA::Object to Notifier is done using the <narrow> method.
    this->notifier_ = IDL::traits<Notifier>::narrow (notifier_object);
  }
  catch (const CORBA::SystemException& sysex)
  {
    taox11_error << "System Exception : Supplier::init(): " << sysex << std::endl;
    return -1;
  }
  catch (const CORBA::UserException& userex)
  {
    taox11_error << "User Exception : Supplier::init(): " << userex << std::endl;
    return -1;
  }

  return 0;
}

ACE_Reactor* Supplier::reactor_used ()
{
  return ACE_Reactor::instance ();
}

// The stock market information is read from a file.
int Supplier::read_file (char* filename)
{
  f_ptr_ = ACE_OS::fopen (filename, "r");

  taox11_debug << "filename = " << filename << std::endl;

  // the stock values are to be read from a file.
  if (f_ptr_ == nullptr)
  {
    ACE_ERROR_RETURN ((LM_ERROR, "Unable to open %s for writing: %p\n", filename), -1);
  }

  return 0;
}
