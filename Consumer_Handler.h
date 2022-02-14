/* -*- C++ -*- */

//=============================================================================
/**
 *  @file    Consumer_Handler.h
 *
 *  Definition of the Callback_Qouter Consumer Client class, Consumer_Handler.
 *
 *  @author Kirthika Parameswaran <kirthika@cs.wustl.edu>
 */
//=============================================================================

#ifndef CONSUMER_HANDLER_H
#define CONSUMER_HANDLER_H

#if !defined(ACE_LACKS_PRAGMA_ONCE)
#  pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include "ConsumerC.h"
#include "Consumer_Input_Handler.h"
#include "Consumer_Signal_Handler.h"
#include "Consumer_i.h"
#include "Naming_Client.h"
#include "NotifierC.h"

#include "ace/Read_Buffer.h"

class Consumer_Input_Handler;
class Consumer_Signal_Handler;

/**
 * @class Consumer_Handler
 *
 * @brief Callback Quoter Consumer Client class.
 *
 * Connects to the Callback Quoter server and
 * registers the Consumer object with the it
 * and receives the stock status from the Notifier.
 */
class Consumer_Handler
{
public:
  /// Constructor.
  Consumer_Handler ();

  /// Destructor.
  ~Consumer_Handler ();

  /// Initialize the client communication with the server.
  int init (int argc, ACE_TCHAR* argv[]);

  /// Start the ORB object.
  int run ();

  /// the name of the stock the consumer is interested in.
  std::string stock_name_;

  /// the desired price of the stock.
  int threshold_value_;

  /// Server object ptr.
  IDL::traits<Notifier>::ref_type server_;

  /// The consumer object.
  Consumer_i* consumer_servant_;

  /// Pointer to the consumer object registered with the ORB.
  IDL::traits<Callback_Quoter::Consumer>::ref_type consumer_var_;

  /// returns the TAO instance of the singleton Reactor.
  static ACE_Reactor* reactor_used ();

  /// Flag which notes whether the consumer has got registered with the
  /// Notifier-server.
  int registered_;

  /// Flag which notes whether the consumer has got unregistered from
  /// the Notifier-server.
  int unregistered_;

private:
  /// Our orb.
  IDL::traits<CORBA::ORB>::ref_type orb_;

  /// Function to read the server IOR from a file.
  int read_ior (ACE_TCHAR* filename);

  /// Parse the command line arguments.  Returns 0 on success, -1 on
  /// error.
  int parse_args ();

  /// This method initialises the naming service and registers the
  /// object with the POA.
  int via_naming_service ();

  /// # of arguments on the command line.
  int argc_;

  /// arguments from command line.
  ACE_TCHAR** argv_;

  /// IOR of the obj ref of the server.
  ACE_TCHAR* ior_;

  /// Flag for server shutdown.
  int shutdown_;

  /// An instance of the name client used for resolving the factory
  /// objects.
  // XXX TAO_Naming_Client naming_services_client_;

  /// This variable denotes whether the naming service
  /// is used or not.
  int use_naming_service_;

  /// Reference to the input_event_handler.
  Consumer_Input_Handler* consumer_input_handler_;

  /// Reference to the signal_event_handler.
  Consumer_Signal_Handler* consumer_signal_handler_;

  /// Is the example interactive?
  int interactive_;
};

#endif /* CONSUMER_HANDLER_H */
