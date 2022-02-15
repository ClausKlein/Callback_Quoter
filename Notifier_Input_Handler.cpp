
//=============================================================================
/**
 *  @file    Notifier_Input_Handler.cpp
 *
 *  Implementation of the callback quoter Notifier_Input_Handler class.
 *
 *  @author Kirthika Parameswaran <kirthika@cs.wustl.edu>
 */
//=============================================================================

#include "Notifier_Input_Handler.h"

#include "tao/x11/log.h"

#include "ace/Get_Opt.h"
#include "ace/OS_NS_ctype.h"

#include <fstream>

// Constructor.
Notifier_Input_Handler::Notifier_Input_Handler ()
  : ior_output_file_ ()
  , argc_ (0)
  , argv_ (0)
  , using_naming_service_ (1)
{}

// Destructor.
Notifier_Input_Handler::~Notifier_Input_Handler ()
{
  // FIXME: Make sure to cleanup the STDIN handler.

#if 0
    if (ACE_Event_Handler::remove_stdin_handler(this->notifier_i_.orb_->orb_core()->reactor(),
            this->notifier_i_.orb_->orb_core()->thr_mgr()) == -1)
    {
        ACE_ERROR((LM_ERROR, "%p\n", "remove_stdin_handler"));
    }
#endif

  taox11_error << "remove_stdin_handler failed" << std::endl;
}

// The naming service is initialized and the naming context as well as
// the object name is bound to the naming server.
int Notifier_Input_Handler::init_naming_service ()
{

#if 1
  // Get reference to initial naming context.
  IDL::traits<CosNaming::NamingContext>::ref_type inc =
    resolve_init<CosNaming::NamingContext> (this->notifier_i_.orb (), "NameService");

  try
  {
    // TODO: check this! CK
    // register the Notifier to the Naming Service.
    CosNaming::Name n (1);
    n[0].id ("Notifier");

    try
    {
      IDL::traits<CosNaming::NamingContext>::ref_type nc = inc->bind_new_context (n);
    }
    catch (const CosNaming::NamingContext::AlreadyBound&)
    {
      // Fine, Notifer context already exists.
      taox11_debug << "Notifer context already exists" << std::endl;
    }

    IDL::traits<Notifier>::ref_type notifier_obj = notifier_i_._this ();

    // Force binding of controller reference to make sure it is always up-to-date.
    inc->rebind (n, notifier_obj);
  }
  catch (const CosNaming::NamingContext::NotFound& ex)
  {
    taox11_error << "Unable to bind Notifier to Naming Service" << ex << std::endl;
    return -1;
  }
#else
  IDL::traits<CORBA::ORB>::ref_type orb = this->orb_manager_.orb ();

  if (this->naming_server_.init (orb) == -1)
  {
    return -1;
  }

  // create the name for the naming service
  CosNaming::Name notifier_obj_name (1);
  notifier_obj_name.length (1);
  notifier_obj_name[0].id = std::string ("Notifier");

  // (re)Bind the object.
  try
  {
    IDL::traits<Notifier>::ref_type notifier_obj = notifier_i_._this ();

    this->orb_manager_.activate_poa_manager ();

    naming_server_->rebind (notifier_obj_name, notifier_obj);
  }
  catch (const CosNaming::NamingContext::AlreadyBound&)
  {
    ACE_ERROR_RETURN ((LM_ERROR, "Unable to bind %s\n", "Notifier"), -1);
  }
#endif

  return 0;
}

// Parse the command-line arguments and set options.
int Notifier_Input_Handler::parse_args ()
{
  ACE_Get_Opt get_opts (this->argc_, this->argv_, ACE_TEXT ("df:s "));
  int c;

  while ((c = get_opts ()) != -1)
  {
    switch (c)
    {
      case 'd': // debug flag.
        // XXX TAO_debug_level++; ///*****
        break;

      case 'f': // output the IOR to a file.
        this->ior_output_file_ = get_opts.opt_arg ();
        break;

      case 's': // don't use the naming service
        this->using_naming_service_ = 0;
        break;

      case '?': // display help for use of the server.
      default:
        ACE_ERROR_RETURN ((LM_ERROR,
                           "usage:  %s"
                           " [-d]"
                           " [-f] <ior_output_file>"
                           " [-s]"
                           "\n",
                           argv_[0]),
                          1);
    }
  }

  // Indicates successful parsing of command line.
  return 0;
}

// Initialize the server.
int Notifier_Input_Handler::init (int argc, char* argv[])
{
  this->argc_ = argc;
  this->argv_ = argv;

#if 1
  // IDL::traits<CORBA::ORB>::ref_type
  auto orb = CORBA::ORB_init (this->argc_, this->argv_);
  if (orb == nullptr)
  {
    taox11_error << "ERROR: CORBA::ORB_init (argc, argv) returned null ORB." << std::endl;
    return -1;
  }

  // Get reference to Root POA.
  // IDL::traits<CORBA::Object>::ref_type
  auto obj = orb->resolve_initial_references ("RootPOA");
  if (!obj)
  {
    taox11_error << "ERROR: resolve_initial_references (\"RootPOA\") returned null reference." << std::endl;
    return -1;
  }

  taox11_info << "retrieved RootPOA object reference" << std::endl;
  // IDL::traits<PortableServer::POA>::ref_type
  auto root_poa = IDL::traits<PortableServer::POA>::narrow (obj);
  if (!root_poa)
  {
    taox11_error << "ERROR: IDL::traits<PortableServer::POA>::narrow (obj) returned null object." << std::endl;
    return -1;
  }

  taox11_info << "narrowed POA interface" << std::endl;
  // IDL::traits<PortableServer::POAManager>::ref_type
  auto poaman = root_poa->the_POAManager ();
  if (!poaman)
  {
    taox11_error << "ERROR: root_poa->the_POAManager () returned null object." << std::endl;
    return -1;
  }

  // CORBA::servant_traits<Notifier>::ref_type
  auto notifier_impl = CORBA::make_reference<Notifier_i> (orb);
  taox11_info << "created Notifier servant" << std::endl;

  // PortableServer::ObjectId
  auto id = root_poa->activate_object (notifier_impl);
  taox11_info << "activated Notifier servant" << std::endl;

  // IDL::traits<CORBA::Object>::ref_type
  auto notifier_obj = root_poa->id_to_reference (id);
  if (notifier_obj == nullptr)
  {
    taox11_error << "ERROR: root_poa->id_to_reference (id) returned null reference." << std::endl;
    return -1;
  }

  // IDL::traits<Notifier>::ref_type
  auto notifier = IDL::traits<Notifier>::narrow (notifier_obj);
  if (notifier == nullptr)
  {
    taox11_error << "ERROR: IDL::traits<Notifier>::narrow (notifier_obj) returned null reference." << std::endl;
    return -1;
  }

  // Activate the servant in the POA.
  poaman->activate ();

  std::string ior = orb->object_to_string (notifier);

#else

  // Call the init of <TAO_ORB_Manager> to initialize the ORB and create the child poa under the root POA.
  if (this->orb_manager_.init_child_poa (this->argc_, this->argv_, "child_poa") == -1)
  {
    ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "init_child_poa"), -1);
  }

  // Register our <Input_Handler> to handle STDIN events, which will trigger the <handle_input> method to process these
  // events.
  IDL::traits<CORBA::ORB>::ref_type orb = this->orb_manager_.orb ();
  if (ACE_Event_Handler::register_stdin_handler (this, orb->orb_core ()->reactor (), orb->orb_core ()->thr_mgr ()) == -1)
  {
    ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "register_stdin_handler"), -1);
  }

  auto ior = this->orb_manager_.activate_under_child_poa ("Notifier", &this->notifier_i_);
#endif

  // TODO TBD: still needed? CK
  // Stash our ORB pointer for later reference.
  this->notifier_i_.orb (orb);

  int retval = this->parse_args ();
  if (retval != 0)
  {
    return retval;
  }

  taox11_debug << "The IOR is: " << ior << std::endl;

  // Output the IOR to the ior_output_file
  std::ofstream fos (this->ior_output_file_);
  if (!fos)
  {
    taox11_error << "ERROR: failed to open file: " << this->ior_output_file_ << std ::endl;
    return -1;
  }

  fos << ior;
  fos.close ();

  if (this->using_naming_service_)
  {
    return this->init_naming_service ();
  }

  return 0;
}

int Notifier_Input_Handler::run ()
{
  // Run the main event loop for the ORB.

  taox11_debug << " Type \"q\" to quit" << std::endl;

#if 1
  this->notifier_i_.orb ()->run ();
#else
  int result = this->orb_manager_.run ();
  if (result == -1)
  {
    ACE_ERROR_RETURN ((LM_ERROR, "Notifier_Input_Handler::run"), -1);
  }
#endif

  return 0;
}

int Notifier_Input_Handler::handle_input (ACE_HANDLE)
{
  char buf[BUFSIZ];

  try
  {
    // The string could read contains \n\0 hence using ACE_OS::read which returns the no of bytes read and hence i can
    // manipulate and remove the devil from the picture i.e '\n' ! ;)

    ssize_t strlen = ACE_OS::read (ACE_STDIN, buf, sizeof buf);
    if (buf[strlen - 1] == '\n')
    {
      buf[strlen - 1] = '\0';
    }

    taox11_debug << buf << std::endl;

    if (ACE_OS::ace_tolower (buf[0]) == 'q')
    {
      // @@ Please remove this call if it's not used.
      // FIXME: (this->notifier_i_.consumer_map_).close();
      this->notifier_i_.shutdown ();
    }
  }
  catch (const CORBA::Exception& ex)
  {
    taox11_error << "Exception in Notifier_Input_Handler::handle_input(): " << ex << std::endl;
    return -1;
  }

  return 0;
}
