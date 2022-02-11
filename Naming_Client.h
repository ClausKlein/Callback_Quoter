#ifndef NAMING_CLIENT_H
#define NAMING_CLIENT_H

#if !defined(ACE_LACKS_PRAGMA_ONCE)
#  pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include "orbsvcs/orbsvcs/naming_server/CosNamingC.h"

#include "tao/x11/log.h"

namespace
{

//----------------------------------------------------------------

template <class T>
typename IDL::traits<T>::ref_type resolve_init (IDL::traits<CORBA::ORB>::ref_type orb, const std::string& id)
{
  IDL::traits<CORBA::Object>::ref_type obj;
  try
  {
    obj = orb->resolve_initial_references (id);
  }
  catch (const CORBA::ORB::InvalidName& e)
  {
    taox11_error << "Caught an unexpected InvalidName exception " << id << ": " << e << std::endl;
    throw;
  }
  catch (const CORBA::Exception& e)
  {
    taox11_error << "Cannot get initial reference for " << id << ": " << e << std::endl;
    throw;
  }

  typename IDL::traits<T>::ref_type ref;
  try
  {
    ref = IDL::traits<T>::narrow (obj);
  }
  catch (const CORBA::Exception& e)
  {
    taox11_error << "Cannot narrow reference for " << id << ": " << e << std::endl;
    throw;
  }
  if (!ref)
  {
    taox11_error << "Incorrect type of reference for " << id << std::endl;
    throw CORBA::INV_OBJREF ();
  }
  return ref;
}

//----------------------------------------------------------------

template <class T>
typename IDL::traits<T>::ref_type resolve_name (IDL::traits<CosNaming::NamingContext>::ref_type nc,
                                                const CosNaming::Name& name)
{
  IDL::traits<CORBA::Object>::ref_type obj;
  try
  {
    obj = nc->resolve (name);
  }
  catch (const CosNaming::NamingContext::NotFound&)
  {
    taox11_error << "NotFound exception caught -> rethrowing" << std::endl;
    throw;
  }
  catch (const CORBA::Exception& e)
  {
    taox11_error << "Cannot resolve binding: " << e << std::endl;
    throw;
  }
  if (!obj)
  {
    taox11_error << "Nil binding in Naming Service" << std::endl;
    throw CORBA::INV_OBJREF ();
  }

  typename IDL::traits<T>::ref_type ref;
  try
  {
    ref = IDL::traits<T>::narrow (obj);
  }
  catch (const CORBA::Exception& e)
  {
    taox11_error << "Cannot narrow reference : " << e << std::endl;
    throw;
  }
  if (!ref)
  {
    taox11_error << "Reference has incorrect type" << std::endl;
    throw CORBA::INV_OBJREF ();
  }
  return ref;
}

//----------------------------------------------------------------

} // namespace
#endif // NAMING_CLIENT_H
