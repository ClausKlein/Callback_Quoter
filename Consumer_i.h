/* -*- C++ -*- */

//=============================================================================
/**
 *  @file    Consumer_i.h
 *
 *  Defines the implementation header for the Consumer interface.
 *
 *  @author Kirthika Parameswaran <kirthika@cs.wustl.edu>
 */
//=============================================================================

#ifndef CONSUMER_I_H
#define CONSUMER_I_H

#if !defined(ACE_LACKS_PRAGMA_ONCE)
#  pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include "ConsumerC.h"
#include "ConsumerS.h"
#include "NotifierC.h"

/**
 * @class Consumer_i
 *
 * @brief Consumer object implementation.
 *
 * This class has methods that are called by the callback quoter server.
 */
class Consumer_i final
  : public virtual CORBA::servant_traits<Callback_Quoter::Consumer>::base_type // FIXME: TBD : public
                                                                               // Callback_Quoter::POA::Consumer
{
public:
  /// Constructor.
  Consumer_i ();

  /// Destructor.
  ~Consumer_i () = default;

  /// Gets the stock information from the Notifier.
  void push (const Callback_Quoter::Info& data) override;

  /// Used to get the consumer to shut down.
  void shutdown () override;

  /// Set the ORB pointer.
  void orb (IDL::traits<CORBA::ORB>::ref_type o);

private:
  /// ORB pointer.
  IDL::traits<CORBA::ORB>::ref_type orb_;

  /// If 1 denotes that the consumer is dead else alive.
  int quit_;

  // @@ Please rename to Notifier.
  /// Smart pointer to the Notifier object.
  IDL::traits<Notifier>::ref_type server_;
};

#endif /* CONSUMER_I_H  */
