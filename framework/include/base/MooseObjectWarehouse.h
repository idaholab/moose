/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef MOOSEOBJECTWAREHOUSE_H
#define MOOSEOBJECTWAREHOUSE_H

// MOOSE includes
#include "MooseObjectWarehouseBase.h"

/**
 * A storage container for MooseObjects that inherit from SetupInterface.
 *
 * Objects that inherit from SetupInterface have various functions (e.g., initialSetup). This
 * class provides convenience functions for looping over all active objects stored in the warehouse
 * and
 * calling the setup methods.
 */
template <typename T>
class MooseObjectWarehouse : public MooseObjectWarehouseBase<T>
{
public:
  using MooseObjectWarehouseBase<T>::checkThreadID;
  using MooseObjectWarehouseBase<T>::_active_objects;
  using MooseObjectWarehouseBase<T>::hasActiveBlockObjects;
  using MooseObjectWarehouseBase<T>::getActiveBlockObjects;

  /**
   * Constructor.
   * @param threaded When true (default) threaded storage is enabled.
   */
  MooseObjectWarehouse(bool threaded = true);

  ///@{
  /**
   * Convenience methods for calling object setup methods.
   */
  virtual void initialSetup(THREAD_ID tid = 0) const;
  virtual void timestepSetup(THREAD_ID tid = 0) const;
  virtual void subdomainSetup(THREAD_ID tid = 0) const;
  virtual void subdomainSetup(SubdomainID id, THREAD_ID tid = 0) const;
  virtual void jacobianSetup(THREAD_ID tid = 0) const;
  virtual void residualSetup(THREAD_ID tid = 0) const;
  ///@}
};

template <typename T>
MooseObjectWarehouse<T>::MooseObjectWarehouse(bool threaded /*=true*/)
  : MooseObjectWarehouseBase<T>(threaded)
{
}

template <typename T>
void
MooseObjectWarehouse<T>::initialSetup(THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  for (const auto & object : _active_objects[tid])
    object->initialSetup();
}

template <typename T>
void
MooseObjectWarehouse<T>::timestepSetup(THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  for (const auto & object : _active_objects[tid])
    object->timestepSetup();
}

template <typename T>
void
MooseObjectWarehouse<T>::subdomainSetup(SubdomainID id, THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  if (hasActiveBlockObjects(id, tid))
  {
    const auto & objects = getActiveBlockObjects(id, tid);
    for (const auto & object : objects)
      object->subdomainSetup();
  }
}

template <typename T>
void
MooseObjectWarehouse<T>::subdomainSetup(THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  for (const auto & object : _active_objects[tid])
    object->subdomainSetup();
}

template <typename T>
void
MooseObjectWarehouse<T>::jacobianSetup(THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  for (const auto & object : _active_objects[tid])
    object->jacobianSetup();
}

template <typename T>
void
MooseObjectWarehouse<T>::residualSetup(THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  for (const auto & object : _active_objects[tid])
    object->residualSetup();
}

#endif // MOOSEOBJECTWAREHOUSE_H
