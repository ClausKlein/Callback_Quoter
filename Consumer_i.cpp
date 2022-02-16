
//=============================================================================
/**
 *  @file    Consumer_i.cpp
 *
 *  Implements the Consumer_i class, which is used by the
 *  callback quoter client.
 *
 *  @author Kirthika Parameswaran <kirthika@cs.wustl.edu>
 */
//=============================================================================

#include "Consumer_i.h"

#include "tao/x11/log.h"

Consumer_i::Consumer_i (IDL::traits<CORBA::ORB>::ref_type orb)
  : quit_ (0)
{
  this->orb_ = std::move (orb);
}

void Consumer_i::push (const Callback_Quoter::Info& data)
{
  // On getting the needed information you now proceed to the next step, which could be obtaining the shares.

  taox11_debug << "Selling 10,000 " << data.stock_name () << " shares at " << data.value () << std::endl;
}

void Consumer_i::shutdown ()
{
  // Instruct the ORB to shutdown.

  taox11_debug << "consumer shutting down" << std::endl;

  this->orb_->shutdown ();
  quit_ = 1;
}
