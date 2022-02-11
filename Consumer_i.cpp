
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

Consumer_i::Consumer_i (): quit_(0)
{}

Consumer_i::~Consumer_i ()
{}

void Consumer_i::push (const Callback_Quoter::Info& data) const
{
  // On getting the needed information you now proceed to the next
  // step, which could be obtaining the shares.

  // TODO ACE_DEBUG ((LM_DEBUG, "Selling 10,000 %s shares at %d!!\n",
  // data.stock_name, data.value));
}

void Consumer_i::shutdown ()
{

  // Instruct the ORB to shutdown.

  // TODO ACE_DEBUG ((LM_DEBUG, " consumer shutting down \n"));

  this->orb_->shutdown ();
  quit_ = 1;
}

void Consumer_i::orb (IDL::traits<CORBA::ORB>::ref_type o)
{
  // Makes a copy of the ORB pointer.

  this->orb_ = std::move (o);
}
