/* -*- C++ -*- */

//=============================================================================
/**
 *  @file    Supplier_Timer_Handler.h
 *
 *  Definition of the Supplier_Timer_Handler class.
 *
 *  @author Kirthika Parameswaran <kirthika@cs.wustl.edu>
 */
//=============================================================================

#ifndef SUPPLIER_TIMER_HANDLER_H
#define SUPPLIER_TIMER_HANDLER_H

#if !defined(ACE_LACKS_PRAGMA_ONCE)
#  pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include "Supplier_i.h"

#include "ace/Event_Handler.h"
#include "ace/Reactor.h"
#include "ace/Timer_Queue.h"

class Supplier;

/**
 * @class Supplier_Timer_Handler
 *
 * @brief Feeds stock information to the Callback Quoter notifier
 * periodically.
 * = Description
 * Create a class to handle timer events.  Since only timer events
 * need to be handled, only the handle_timeout method is overlaoded.
 */
class Supplier_Timer_Handler : public ACE_Event_Handler
{
public:
  /// Initilization.
  Supplier_Timer_Handler (Supplier* supplier, ACE_Reactor* reactor, FILE* file_ptr);

  /// Destructor.
  ~Supplier_Timer_Handler () = default;

  /// Method which will be called by the Reactor when timeout occurs.
  int handle_timeout (const ACE_Time_Value& tv, const void* arg = 0) override;

private:
  /// The values of the stock and its rate are got from the file.
  int get_stock_information ();

  /// The supplier instance.
  Supplier* supplier_obj_;

  /// Reactor used by the supplier.
  ACE_Reactor* reactor_;

  /// The file handle of the file from where the stock input is obtained.
  FILE* file_ptr_;

  /// The name of the stock.
  char stockname_[BUFSIZ] {};

  /// The market value of the stock.It will be typecasted to long later.
  int32_t value_ {-1};
};

#endif /* SUPPLIER_TIMER_HANDLER_H */
