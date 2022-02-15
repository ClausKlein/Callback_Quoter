
//=============================================================================
/**
 *  @file    Notifier_i.cpp
 *
 * Implementation of the Notifier_i class. This class is the servant
 * object for the callback quoter server.
 *
 *  @author Kirthika Parameswaran <kirthika@cs.wustl.edu>
 */
//=============================================================================

#include "Notifier_i.h"

#include "tao/x11/log.h"

Notifier_i::Notifier_i ()
  : notifier_exited_ (0)
{
  // No-op
}

// Register a distributed callback handler that is invoked when the given stock reaches the desired threshold value.
void Notifier_i::register_callback (const std::string& stock_name,
                                    int32_t threshold_value,
                                    IDL::traits<Callback_Quoter::Consumer>::ref_type consumer_handler)
{
  // Store the client information.
  Consumer_Data consumer_data;
  consumer_data.consumer_ = std::move (consumer_handler);
  consumer_data.desired_value_ = threshold_value;

  // The consumer_map consists of the stockname and various consumers with their threshold values. To register a consumer
  // into this map, first the stockname is matched with an existing one (if any) and the consumer and the threshold value
  // is attached. Else, a new entry is created for the stockname.

  auto consumers = this->consumer_map_.find (stock_name);
  if (consumers != this->consumer_map_.end ())
  { // found a set entry at the map
    if (!consumers->second.insert (consumer_data).second)
    {
      throw Callback_Quoter::Invalid_Stock ("Insertion failed! Invalid Stock!\n");
    }

    taox11_debug << "Inserted map entry: stockname " << stock_name << " threshold " << threshold_value << std::endl;
  }
  else
  {
    // a set entry is created.
    CONSUMERS consumers;

    // When a new entry is tried to be inserted into the unbounded set and it
    // fails an exception is raised.
    if (!consumers.insert (consumer_data).second)
    {
      throw Callback_Quoter::Invalid_Stock ("Insertion failed! Invalid Stock!\n");
    }

    // The bond between the stockname <hash_key> and the consumers <hash_value>
    // is fused.
    auto result = this->consumer_map_.insert (std::make_pair (stock_name, consumers));
    if (!result.second)
    {
      taox11_error << "register_callback: Bind failed!" << std::endl;
    }
    else
    {
      taox11_debug << "new map entry: stockname " << stock_name << " threshold " << threshold_value << std::endl;
    }
  }
}

// Set a reference to the orb.
void Notifier_i::orb (CORBA::ORB::_ref_type orb)
{
  this->orb_ = std::move (orb);
}

// Obtain the orb reference.
CORBA::ORB::_ref_type Notifier_i::orb ()
{
  return this->orb_;
}

// Remove the client handler.
void Notifier_i::unregister_callback (IDL::traits<Callback_Quoter::Consumer>::ref_type consumer)
{
  // The consumer_map consists of a map of stocknames with consumers stock_name.c_str(), threshold_value)); and their
  // threshold values attached to it. To unregister a consumer it is necessary to remove that entry from the map. Hence,
  // the map is iterated till the consumer entry to be removed is found and then removed from the map.

  // Check to see whether the hash_map still exists. Chances are there that the notifier has exited closing the hash map.
  if (notifier_exited_ == 1)
  {
    return;
  }

  Consumer_Data consumer_to_remove;
  consumer_to_remove.consumer_ = std::move (consumer);

  for (auto& iter : this->consumer_map_)
  {
    // The iter is nothing but the stockname + unbounded set of consumers+threshold values, i.e a Hash_Map_Entry.

    if (iter.second.erase (consumer_to_remove) == 0)
    {
      throw Callback_Quoter::Invalid_Handle ("Unregistration failed! Invalid Consumer Handle!\n");
    }

    taox11_debug << "unregister_callback: consumer removed" << std::endl;
  }
}

// Gets the market status and sends the information to the consumer who is interested in it.
void Notifier_i::market_status (const std::string& stock_name, int32_t stock_value)
{
  taox11_debug << "Notifier_i::market_status: The stockname is " << stock_name << " with price " << stock_value
               << std::endl;

  // XXX CONSUMER_MAP::iterator consumers = this->consumer_map_.find (stock_name);
  auto consumers = this->consumer_map_.find (stock_name);
  if (consumers != this->consumer_map_.end ())
  {
    // Go through the list of <Consumer_Data> to find which registered client wants to be notified.

    // XXX for (Consumer_Data& item : this->consumer_map_[stock_name])
    // NOT possible! CK (aka 'const Notifier_i::Consumer_Data') drops 'const' qualifier
    for (auto item : consumers->second)
    {
      // Check whether the stockname is equal before proceeding further.
      if (stock_value >= item.desired_value_)
      {
        const Callback_Quoter::Info interested_consumer_data (stock_name, stock_value);

        taox11_debug << "pushing information to consumer" << std::endl;

        // The status desired by the consumer is then passed to it.
        item.consumer_->push (interested_consumer_data);
      }
    }
  }
  else
  {
    taox11_debug << "Stock Not Present!" << std::endl;
  }

  // Raising an exception caused problems as they were caught by the Market daemon who exited prematurely.
}

void Notifier_i::shutdown ()
{
  this->consumer_map_.clear ();

  // This marks the exit of the notifier. This should be taken care of
  // before the consumer tries to unregister after the notifier quits.
  notifier_exited_ = 1;

  taox11_debug << "The Callback Quoter server is shutting down..." << std::endl;

  // Instruct the ORB to shutdown.
  this->orb_->shutdown ();
}

bool Notifier_i::Consumer_Data::operator== (const Consumer_Data& rhs) const
{
  // NOTE: this call might not behave well on other ORBs
  // since <_is_equivalent> isn't guaranteed to differentiate object references.
  return this->consumer_->_is_equivalent (rhs.consumer_);
}

bool Notifier_i::Consumer_Data::operator< (const Consumer_Data& rhs) const
{
  return this->desired_value_ < rhs.desired_value_;
}
