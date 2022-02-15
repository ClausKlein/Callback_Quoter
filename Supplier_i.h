// -*- C++ -*-
//=============================================================================
/**
 *  @file    Supplier_i.h
 *
 *  This class implements a simple CORBA server that keeps
 *   on sending stock values to the Notifier.
 *
 *  @author Kirthika Parameswaran <kirthika@cs.wustl.edu>
 */
//=============================================================================

#ifndef SUPPLIER_I_H
#define SUPPLIER_I_H

#if !defined(ACE_LACKS_PRAGMA_ONCE)
#  pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include "Naming_Client.h"
#include "NotifierC.h"
#include "Supplier_Timer_Handler.h"

#include "ace/Reactor.h"
#include "ace/Read_Buffer.h"

/**
 * @class Supplier
 *
 * @brief Market feed  daemon implementation.
 *
 * This class feeds stock information to the Callback Quoter
 * notifier.
 */
class Supplier_Timer_Handler;
class Supplier
{
public:
  /// Constructor.
  Supplier ();

  /// Destructor.
  ~Supplier ();

  /// Execute the daemon.
  int run () const;

  /// Initialize the client communication endpoint with Notifier.
  int init (int argc, char* argv[]);

  /// Sends the stock name and its value.
  int send_market_status (const char* stock_name, int32_t value);

  /// The timer handler used to send the market status to the notifier
  /// periodically.
  Supplier_Timer_Handler* supplier_timer_handler_;

private:
  /// Remember our orb.
  IDL::traits<CORBA::ORB>::ref_type orb_;

  /// Function to read the Notifier IOR from a file.
  int read_ior (const std::string& filename);

  /// Parses the arguments passed on the command line.
  int parse_args ();

  /// This method initialises the naming service and registers the
  /// object with the POA.
  int via_naming_service ();

  /// returns the TAO instance of the singleton Reactor.
  static ACE_Reactor* reactor_used ();

  /// This method used for getting stock information from a file.
  int read_file (char* filename);

  /// # of arguments on the command line.
  int argc_;

  /// arguments from command line.
  char** argv_;

  /// IOR of the obj ref of the Notifier.
  std::string ior_;

  /// An instance of the name client used for resolving the factory
  /// objects.
  // XXX TAO_Naming_Client naming_services_client_;

  /// This variable denotes whether the naming service
  /// is used or not.
  int use_naming_service_;

  /// Notifier object reference.
  IDL::traits<Notifier>::ref_type notifier_;

  /// The pointer for accessing the input stream.
  FILE* f_ptr_;

  /// Iteration count.
  // UNUSED! int loop_count_;

  /// Time period between two succesive market feeds to the Notifier.
  long period_value_;
};

#endif /*SUPPLIER_I_H */
