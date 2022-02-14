/* -*- C++ -*- */
//=============================================================================
/**
 *  @file    Notifier_Input_Handler.h
 *
 *  Definition of the Callback_Quoter Notifier_Input_Handler class.
 *
 *  @author Kirthika Parameswaran <kirthika@cs.wustl.edu>
 */
//=============================================================================

#ifndef SUPPLIER_INPUT_HANDLER_H
#define SUPPLIER_INPUT_HANDLER_H

#if !defined(ACE_LACKS_PRAGMA_ONCE)
#  pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include "Naming_Client.h"
#include "NotifierS.h"
#include "Notifier_i.h"

#include "ace/Event_Handler.h"

/**
 * @class Notifier_Input_Handler
 *
 * @brief The class defines the callback quoter Notifier initialization
 * and run methods.
 *
 * This class handles initialization tasks, as well, such as
 * setting up the Orb manager and registers the Notifier servant
 * object.
 */
class Notifier_Input_Handler : public ACE_Event_Handler
{
public:
  /// Constructor.
  Notifier_Input_Handler ();

  /// Destructor.
  ~Notifier_Input_Handler ();

  /// Initialize the Notifier who plays the role of the server here.
  int init (int argc, ACE_TCHAR* argv[]);

  /// Run the ORB.
  int run ();

  /// Handle the user input.
  virtual int handle_input (ACE_HANDLE);

private:
  /// Parses the command line arguments.
  int parse_args ();

  /// Initialises the name server and registers the Notifier object
  /// name with the name server.
  int init_naming_service ();

  /// File where the IOR of the Notifier object is stored.
  FILE* ior_output_file_;

  /// Number of command line arguments.
  int argc_;

  /// The command line arguments.
  ACE_TCHAR** argv_;

  /// Naming context for the naming service.
  IDL::traits<CosNaming::NamingContext>::ref_type naming_context_;

  /// The servant object registered with the orb.
  Notifier_i notifier_i_;

  /// This specifies whether the naming service is to be used.
  int using_naming_service_;
};

#endif /* NOTIFIER_INPUT_HANDLER_H */
