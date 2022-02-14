/* -*- C++ -*- */

//=============================================================================
/**
 *  @file    Notifier_i.h
 *
 *  Defines the implementation header for the Supplier interface.
 *
 *  @author Kirthika Parameswaran <kirthika@cs.wustl.edu>
 */
//=============================================================================

#ifndef NOTIFIER_I_H
#define NOTIFIER_I_H

#if !defined(ACE_LACKS_PRAGMA_ONCE)
#  pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include "ConsumerC.h"
#include "NotifierS.h"

#include <set>
#include <unordered_map>

/**
 * @class Notifier_i
 *
 * @brief Notifier servant class.
 *
 * The implementation of the Notifier class, which is the servant
 * object for the callback quoter server.
 */
class Notifier_i : public POA::Notifier
{
public:
  /// Constructor.
  Notifier_i ();

  /// Destructor.
  ~Notifier_i () = default;

  /// Register a distributed callback handler that is invoked when the
  /// given stock reaches the desired threshold value.
  void register_callback (const std::string& stock_name,
                          int32_t threshold_value,
                          taox11::CORBA::object_traits<Callback_Quoter::Consumer>::ref_type consumer_handler) override;

  /// Remove the consumer object.
  void unregister_callback (taox11::CORBA::object_traits<Callback_Quoter::Consumer>::ref_type consumer_handler) override;

  /// Get the market status.
  void market_status (const std::string& stock_name, int32_t stock_value) override;

  /// Set the orb pointer.
  void orb (CORBA::ORB::_ref_type orb);

  /// Get the orb pointer.
  CORBA::ORB::_ref_type orb ();

  /// Shutdown the Notifier.
  void shutdown () override;

private:
  /// The ORB manager.
  IDL::traits<CORBA::ORB>::ref_type orb_;

  /**
   * @class Consumer_Data
   *
   * @brief Saves the Consumer_var and the threshold stock value.
   */
  class Consumer_Data
  {
  public:
    /// Comparison operator.
    bool operator== (const Consumer_Data& rhs) const;
    bool operator< (const Consumer_Data& rhs) const;

    /// Stores the consumer object reference.
    IDL::traits<Callback_Quoter::Consumer>::ref_type consumer_;

    /// Stores the stock threshold value.
    int32_t desired_value_;
  };

  typedef std::set<Consumer_Data> CONSUMERS;

  typedef std::unordered_map<std::string, CONSUMERS> CONSUMER_MAP;

  /// This is the hash map with each hash_entry consisting of the stockname
  /// and an unbounded set of consumer object pointer and the desired
  /// stockvalue.
  CONSUMER_MAP consumer_map_;

  /// This marks the exit of the notifier. This should be taken care of
  /// before the consumer tries to unregister after the notifier quits.
  int notifier_exited_;
};

#endif /* NOTIFIER_I_H */
