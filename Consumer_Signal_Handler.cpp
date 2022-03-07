/* -*- C++ -*- */

//=============================================================================
/**
 *  @file    Consumer_Signal_Handler.cpp
 *
 *  Implementation of the Consumer_Signal_Handler class.
 *
 *  @author Kirthika Parameswaran <kirthika@cs.wustl.edu>
 */
//=============================================================================

#include "Consumer_Signal_Handler.h"

#include "tao/x11/log.h"

Consumer_Signal_Handler::Consumer_Signal_Handler (Consumer_Handler* consumer_handler)
  : consumer_handler_ (consumer_handler)
{}

// Method to handle the ^C signal.
int Consumer_Signal_Handler::handle_signal (int /* signum */, siginfo_t*, ucontext_t*)
{
  taox11_debug << "Exiting on receiving ^C" << std::endl;

  quit_on_signal ();

  return 0;
}

// Method called before the Event_Handler dies.
int Consumer_Signal_Handler::handle_close (ACE_HANDLE, ACE_Reactor_Mask)
{
  // End of the signal handler.
  delete this;

  return 0;
}

int Consumer_Signal_Handler::quit_on_signal ()
{
  // Only if the consumer is registered and wants to shut down, its necessary to unregister and then shutdown.

  try
  {
    if (consumer_handler_->unregistered_ != 1 && consumer_handler_->registered_ == 1)
    {
      this->consumer_handler_->server_->unregister_callback (this->consumer_handler_->consumer_var_);
      taox11_debug << "Consumer Unregistered" << std::endl;
    }
    this->consumer_handler_->consumer_var_->shutdown ();
  }
  catch (const CORBA::Exception& ex)
  {
    taox11_error << "Consumer_Input_Handler::quit_on_signal()" << ex << std::endl;
    return -1;
  }

  return 0;
}
