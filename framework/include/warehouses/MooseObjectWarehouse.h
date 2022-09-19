//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseObjectWarehouseBase.h"
#include "MooseVariableInterface.h"
#include "MooseVariableFE.h"

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
  using MooseObjectWarehouseBase<T>::_all_objects;
  using MooseObjectWarehouseBase<T>::_active_objects;
  using MooseObjectWarehouseBase<T>::hasActiveBlockObjects;
  using MooseObjectWarehouseBase<T>::getActiveBlockObjects;

  /**
   * Constructor.
   * @param threaded When true (default) threaded storage is enabled.
   */
  MooseObjectWarehouse(bool threaded = true);

  /**
   * Adds an object to the storage structure.
   * @param object A shared pointer to the object being added
   * @param tid The thread ID (default is 0)
   * @param recurse Whether or not to build recusive warehouses (typically for Kernels)
   */
  virtual void
  addObject(std::shared_ptr<T> object, THREAD_ID tid = 0, bool recurse = true) override;

  ///@{
  /**
   * Methods for checking/getting variable kernels for a variable and SubdomainID
   */
  bool hasActiveVariableBlockObjects(unsigned int variable_id,
                                     SubdomainID block_id,
                                     THREAD_ID tid = 0) const;
  const std::vector<std::shared_ptr<T>> & getActiveVariableBlockObjects(unsigned int variable_id,
                                                                        SubdomainID block_id,
                                                                        THREAD_ID tid = 0) const;
  ///@}

  /**
   * Update the active status of Kernels
   */
  virtual void updateActive(THREAD_ID tid = 0) override;

  /**
   * generic setup function that will forward to the virtual interfaces based on the execution flag
   */
  virtual void setup(const ExecFlagType & exec_type, THREAD_ID tid = 0) const;

  ///@{
  /**
   * Convenience methods for calling object setup methods.
   */
  virtual void subdomainSetup(SubdomainID id, THREAD_ID tid = 0) const;
  ///@}

protected:
  /// Variable based storage
  std::map<unsigned int, MooseObjectWarehouse<T>> _variable_objects;
};

template <typename T>
MooseObjectWarehouse<T>::MooseObjectWarehouse(bool threaded /*=true*/)
  : MooseObjectWarehouseBase<T>(threaded)
{
}

template <typename T>
void
MooseObjectWarehouse<T>::addObject(std::shared_ptr<T> object,
                                   THREAD_ID tid /*= 0*/,
                                   bool recurse /* = true */)
{
  MooseObjectWarehouseBase<T>::addObject(object, tid, recurse);

  if (recurse)
  {
    {
      auto mvi = std::dynamic_pointer_cast<MooseVariableInterface<Real>>(object);

      if (mvi)
        _variable_objects[mvi->mooseVariableBase()->number()].addObject(object, tid, false);
    }

    {
      auto mvi = std::dynamic_pointer_cast<MooseVariableInterface<RealVectorValue>>(object);

      if (mvi)
        _variable_objects[mvi->mooseVariableBase()->number()].addObject(object, tid, false);
    }

    {
      auto mvi = std::dynamic_pointer_cast<MooseVariableInterface<RealEigenVector>>(object);

      if (mvi)
        _variable_objects[mvi->mooseVariableBase()->number()].addObject(object, tid, false);
    }
  }
}

template <typename T>
bool
MooseObjectWarehouse<T>::hasActiveVariableBlockObjects(unsigned int variable_id,
                                                       SubdomainID block_id,
                                                       THREAD_ID tid) const
{
  auto iter = _variable_objects.find(variable_id);
  return (iter != _variable_objects.end() && iter->second.hasActiveBlockObjects(block_id, tid));
}

template <typename T>
const std::vector<std::shared_ptr<T>> &
MooseObjectWarehouse<T>::getActiveVariableBlockObjects(unsigned int variable_id,
                                                       SubdomainID block_id,
                                                       THREAD_ID tid) const
{
  checkThreadID(tid);
  const auto iter = _variable_objects.find(variable_id);
  mooseAssert(iter != _variable_objects.end(),
              "Unable to locate variable kernels for the given variable id: " << variable_id
                                                                              << ".");
  return iter->second.getActiveBlockObjects(block_id, tid);
}

template <typename T>
void
MooseObjectWarehouse<T>::setup(const ExecFlagType & exec_type, const THREAD_ID tid) const
{
  checkThreadID(tid);
  for (const auto & object : _all_objects[tid])
    object->setup(exec_type);
}

template <typename T>
void
MooseObjectWarehouse<T>::subdomainSetup(SubdomainID id, THREAD_ID tid /* = 0*/) const
{
  if (hasActiveBlockObjects(id, tid))
  {
    const auto & objects = getActiveBlockObjects(id, tid);
    for (const auto & object : objects)
      object->setup(EXEC_SUBDOMAIN);
  }
}

template <typename T>
void
MooseObjectWarehouse<T>::updateActive(THREAD_ID tid)
{
  MooseObjectWarehouseBase<T>::updateActive(tid);

  for (auto & it : _variable_objects)
    it.second.updateActive(tid);
}
