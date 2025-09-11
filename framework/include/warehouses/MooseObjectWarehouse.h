//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "ResidualObject.h"

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
  MooseObjectWarehouse(unsigned int num_threads);

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
   * Convenience methods for calling object setup methods.
   */
  virtual void initialSetup(THREAD_ID tid = 0) const;
  virtual void timestepSetup(THREAD_ID tid = 0) const;
  virtual void customSetup(const ExecFlagType & exec_type, THREAD_ID tid = 0) const;
  virtual void subdomainSetup(THREAD_ID tid = 0) const;
  virtual void subdomainSetup(SubdomainID id, THREAD_ID tid = 0) const;
  virtual void jacobianSetup(THREAD_ID tid = 0) const;
  virtual void residualSetup(THREAD_ID tid = 0) const;
  ///@}

  /**
   * Checks for whether this warehouse has objects for a given variable
   */
  bool hasVariableObjects(unsigned int variable_id, THREAD_ID tid = 0) const;

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

protected:
  /// Variable based storage
  std::map<unsigned int, MooseObjectWarehouse<T>> _variable_objects;
};

template <typename T>
MooseObjectWarehouse<T>::MooseObjectWarehouse(unsigned int num_threads)
  : MooseObjectWarehouseBase<T>(num_threads)
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
    if (auto mvir = std::dynamic_pointer_cast<MooseVariableInterface<Real>>(object); mvir)
    {
      _variable_objects.emplace(std::make_pair(mvir->mooseVariableBase()->number(), this->_num_threads));
      _variable_objects.at(mvir->mooseVariableBase()->number()).addObject(object, tid, false);
    }
    else if (auto mviv = std::dynamic_pointer_cast<MooseVariableInterface<RealVectorValue>>(object);
             mviv)
    {
      _variable_objects.emplace(std::make_pair(mviv->mooseVariableBase()->number(), this->_num_threads));
      _variable_objects.at(mviv->mooseVariableBase()->number()).addObject(object, tid, false);
    }
    else if (auto mvie = std::dynamic_pointer_cast<MooseVariableInterface<RealEigenVector>>(object);
             mvie)
    {
      _variable_objects.emplace(std::make_pair(mvie->mooseVariableBase()->number(), this->_num_threads));
      _variable_objects.at(mvie->mooseVariableBase()->number()).addObject(object, tid, false);
    }
    // Some objects, such as ScalarKernels, do not inherit from the MooseVariableInterface (which is
    // for field variables). These objects *do* inherit from ResidualObject which has this more
    // generic variable() API
    else if (auto ro = std::dynamic_pointer_cast<ResidualObject>(object); ro)
    {
      // One liner - less readable?
      (*_variable_objects.emplace(std::make_pair(ro->variable().number(), this->_num_threads)).first).second.addObject(object, tid, false);
    }
  }
}

template <typename T>
bool
MooseObjectWarehouse<T>::hasVariableObjects(const unsigned int variable_id, THREAD_ID tid) const
{
  auto iter = _variable_objects.find(variable_id);
  return (iter != _variable_objects.end() && iter->second.hasObjects(tid));
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
MooseObjectWarehouse<T>::initialSetup(THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  // Initial Setup should be called on all objects because they may become active later
  for (const auto & object : _all_objects[tid])
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
MooseObjectWarehouse<T>::customSetup(const ExecFlagType & exec_type, THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  for (const auto & object : _active_objects[tid])
    object->customSetup(exec_type);
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

template <typename T>
void
MooseObjectWarehouse<T>::updateActive(THREAD_ID tid)
{
  MooseObjectWarehouseBase<T>::updateActive(tid);

  for (auto & it : _variable_objects)
    it.second.updateActive(tid);
}
