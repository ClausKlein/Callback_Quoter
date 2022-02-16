
//=============================================================================
/**
 *  @file    Consumer_Handler.cpp
 *
 *  Implementation of the Consumer_Handler class.
 *
 *  @author Kirthika Parameswaran <kirthika@cs.wustl.edu>
 */
//=============================================================================

#include "Consumer_Handler.h"

#include "Naming_Client.h"

#include "tao/x11/log.h"

#undef USE_ORB_PROXY // FIXME: does not compile! CK
#ifdef USE_ORB_PROXY

#  include "tao/x11/orb.h"

#  include "tao/x11/orbproxy.h" // ORB_Proxy& proxy()
// FIXME: using TAO_3_0_6::CORBA as CORBA;
// FIXME: using TAOX11_NAMESPACE::CORBA as CORBA;
using namespace TAOX11_NAMESPACE;

#endif

#include "ace/Event_Handler.h"
#include "ace/Get_Opt.h"

#include <fstream>

Consumer_Handler::Consumer_Handler ()
  : stock_name_ ("Unknown")
  , threshold_value_ (0)
  , consumer_servant_ (nullptr)
  , registered_ (0)
  , unregistered_ (0)
  , argc_ (0)
  , argv_ (nullptr)
  , ior_ ()
  // UNUSED , shutdown_ (0)
  , use_naming_service_ (1)
  , consumer_input_handler_ (nullptr)
  , consumer_signal_handler_ (nullptr)
  , interactive_ (0)
{}

Consumer_Handler::~Consumer_Handler ()
{
  if (this->interactive_ == 1)
  {
    // Make sure to cleanup the STDIN handler.

    // FIXME: this does not compile! CK
#ifdef USE_ORB_PROXY
    if (ACE_Event_Handler::remove_stdin_handler (reactor_used (), this->orb_->proxy ().orb_core ()->thr_mgr ()) == -1)
    {
      ACE_ERROR ((LM_ERROR, "%p\n", "remove_stdin_handler"));
    }
#else
    taox11_error << "remove_stdin_handler failed" << std::endl;
#endif

    // see Consumer_Signal_Handler::handle_close()
    // NOTE: not needed to delete this->consumer_signal_handler_;
    // see Consumer_Input_Handler::handle_close()
    // NOTE: not needed to delete this->consumer_input_handler_;
  }

  // TODO: check this! CK
  delete this->consumer_servant_;
}

// Reads the Server factory IOR from a file.
int Consumer_Handler::read_ior (const std::string& filename)
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
int Consumer_Handler::parse_args ()
{
  ACE_Get_Opt get_opts (argc_, argv_, ACE_TEXT ("a:t:d:f:k:s"));
  int c;
  int result;

  while ((c = get_opts ()) != -1)
  {
    switch (c)
    {
      case 'd': // debug flag
        // TODO: TAO_debug_level++; //****
        break;

      case 'k': // ior provide on command line
        this->ior_ = get_opts.opt_arg ();
        break;

      case 'f': // read the IOR from the file.
        result = this->read_ior (get_opts.opt_arg ());
        if (result < 0)
        {
          ACE_ERROR_RETURN ((LM_ERROR, "Unable to read ior from %s : %p\n", get_opts.opt_arg ()), -1);
        }
        break;

      case 's': // don't use the naming service
        this->use_naming_service_ = 0;
        break;

      case 'a': // to be given only on using run_test.pl
        this->stock_name_ = ACE_TEXT_ALWAYS_CHAR (get_opts.opt_arg ());
        this->interactive_ = 0;
        break;

      case 't':
        this->threshold_value_ = ACE_OS::atoi (get_opts.opt_arg ());
        break;

        // UNUSED case 'x': this->shutdown_ = 1; break;

      case '?':
      default:
        ACE_ERROR_RETURN ((LM_ERROR,
                           "usage:  %s"
                           " [-d]"
                           " [-f ior-file]"
                           " [-k ior]"
                           // UNUSED [-x]"
                           " [-s]"
                           " [-a stock_name]"
                           " [-t threshold]"
                           "\n",
                           this->argv_[0]),
                          -1);
    }
  }

  // Indicates successful parsing of command line.
  return 0;
}

// this method uses the naming service to obtain the server object refernce.
int Consumer_Handler::via_naming_service ()
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
    this->server_ = IDL::traits<Notifier>::narrow (notifier_obj);
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

    // The CORBA::Object object is downcast to Notifier using the <narrow> method.
    this->server_ = IDL::traits<Notifier>::narrow (notifier_obj);
  }
  catch (const CORBA::Exception& ex)
  {
    taox11_error << "Exception in Consumer_Handler::via_naming_service(): " << std::endl;
    return -1;
  }
#endif

  return 0;
}

// Init function.
int Consumer_Handler::init (int argc, char** argv)
{
  this->argc_ = argc;
  this->argv_ = argv;

  // Register our <Input_Handler> to handle STDIN events, which will trigger the <handle_input> method to process these
  // events.

  try
  {
    // Retrieve the ORB.
    this->orb_ = CORBA::ORB_init (this->argc_, this->argv_);

    // Parse command line and verify parameters.
    if (this->parse_args () == -1)
    {
      ACE_ERROR_RETURN ((LM_ERROR, "parse_args failed\n"), -1);
    }

    if (this->interactive_ == 1)
    {
      taox11_debug << "Services provided:\n * Registration <type 'r'>\n * Unregistration <type 'u'>\n * Quit <type 'q'>\n"
                   << std::endl;

      ACE_NEW_RETURN (consumer_input_handler_, Consumer_Input_Handler (this), -1);

      // FIXME: this does not compile! CK
#ifdef USE_ORB_PROXY
      if (ACE_Event_Handler::register_stdin_handler (
            consumer_input_handler_, reactor_used (), this->orb_->proxy ().orb_core ()->thr_mgr ()) == -1)
      {
        ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "register_stdin_handler"), -1);
      }
#endif

      // Register the signal event handler for ^C
      ACE_NEW_RETURN (consumer_signal_handler_, Consumer_Signal_Handler (this), -1);

      if (reactor_used ()->register_handler (SIGINT, consumer_signal_handler_) == -1)
      {
        ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "register_handler for SIGINT"), -1);
      }
    }

    // use the naming service.
    if (this->use_naming_service_)
    {
      if (via_naming_service () == -1)
      {
        ACE_ERROR_RETURN ((LM_ERROR, "via_naming_service failed\n"), -1);
      }
    }
    else
    {
      if (this->ior_.empty ())
      {
        ACE_ERROR_RETURN ((LM_ERROR, "%s: no ior specified\n", this->argv_[0]), -1);
      }

      IDL::traits<CORBA::Object>::ref_type server_object = this->orb_->string_to_object (this->ior_);

      if (server_object == nullptr)
      {
        ACE_ERROR_RETURN ((LM_ERROR, "invalid ior <%s>\n", this->ior_.c_str ()), -1);
      }

      // The downcasting from CORBA::Object to Notifier is done using the <narrow> method.
      this->server_ = IDL::traits<Notifier>::narrow (server_object);
    }
  }
  catch (const CORBA::Exception& ex)
  {
    taox11_error << "Exception in Consumer_Handler::init(): " << ex << std::endl;
    return -1;
  }

  return 0;
}

int Consumer_Handler::run ()
{
  try
  {
    // Obtain and activate the RootPOA.
    IDL::traits<CORBA::Object>::ref_type obj = this->orb_->resolve_initial_references ("RootPOA");
    IDL::traits<PortableServer::POA>::ref_type root_poa = IDL::traits<PortableServer::POA>::narrow (obj);
    IDL::traits<PortableServer::POAManager>::ref_type poa_manager = root_poa->the_POAManager ();
    poa_manager->activate ();

    // NOTE: this is the old way to create a server with new:
    // TODO: prevent this, use RAII! CK
    ACE_NEW_RETURN (this->consumer_servant_, Consumer_i (this->orb_), -1);

    // Set the orb in the consumer_ object.
    // DONE this->consumer_servant_->orb (this->orb_);

    // Get the consumer stub (i.e consumer object) reverence.
    this->consumer_var_ = this->consumer_servant_->_this ();

    if (this->interactive_ == 0)
    {
      // Register with the server.
      this->server_->register_callback (this->stock_name_, this->threshold_value_, this->consumer_var_);

      // Note the registration.
      this->registered_ = 1;
      this->unregistered_ = 0;

      taox11_debug << "registeration done!" << std::endl;
    }

    // Run the ORB.
    this->orb_->run ();
  }
  catch (const CORBA::Exception& ex)
  {
    taox11_error << "Exception in Consumer_Handler::run(): " << ex << std::endl;
    return -1;
  }

  return 0;
}

ACE_Reactor* Consumer_Handler::reactor_used ()
{
  return ACE_Reactor::instance ();
}
