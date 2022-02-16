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

#include "ConsumerS.h"
#include "NotifierC.h"

/**
 * @class Consumer_i
 *
 * @brief Consumer object implementation.
 *
 * This class has methods that are called by the callback quoter server.
 */
class Consumer_i final : public virtual CORBA::servant_traits<Callback_Quoter::Consumer>::base_type
{
public:
  /// Constructor.
  Consumer_i (IDL::traits<CORBA::ORB>::ref_type orb);

  /// Destructor.
  ~Consumer_i () = default;

  /// Gets the stock information from the Notifier.
  void push (const Callback_Quoter::Info& data) override;

  /// Used to get the consumer to shut down.
  void shutdown () override;

private:
  /// ORB used.
  IDL::traits<CORBA::ORB>::ref_type orb_;

  /// If 1 denotes that the consumer is dead else alive.
  // UNUSED int quit_;

  /// Smart pointer to the Notifier object.
  // UNUSED IDL::traits<Notifier>::ref_type notifier_;
};

#endif /* CONSUMER_I_H  */
