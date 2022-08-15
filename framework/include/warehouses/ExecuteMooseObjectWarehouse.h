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
#include "MooseObjectWarehouse.h"
#include "SetupInterface.h"

/**
 * A class for storing MooseObjects based on execution flag.
 *
 * Note: The global list of objects, those accessed via the "get" methods of this class, are not
 * sorted when the
 *       sort method is called. This is because cyclic errors occur even when the execution flags
 * differ.
 */
template <typename T>
class ExecuteMooseObjectWarehouse : public MooseObjectWarehouse<T>
{
public:
  using MooseObjectWarehouse<T>::checkThreadID;
  using MooseObjectWarehouse<T>::initialSetup;
  using MooseObjectWarehouse<T>::timestepSetup;
  using MooseObjectWarehouse<T>::subdomainSetup;

  /**
   * Constructor.
   * @param threaded True enables threaded object storage (default).
   */
  ExecuteMooseObjectWarehouse(const ExecFlagEnum & flags, bool threaded = true);

  virtual ~ExecuteMooseObjectWarehouse();

  /**
   * Adds an object to the storage structure.
   * @param object A shared pointer to the object being added
   */
  void addObject(std::shared_ptr<T> object, THREAD_ID tid = 0, bool recurse = true) override;

  ///@{
  /**
   * Retrieve shared pointers for the given thread and execution type for all/active objects.
   * @param exec_flag The execution flag to retrieve objects from
   */
  const MooseObjectWarehouse<T> & operator[](ExecFlagType exec_flag) const;
  MooseObjectWarehouse<T> & operator[](ExecFlagType exec_flag);
  ///@}

  ///@{
  /**
   * Provide access to begin/end iterators of the underlying map of execution flags.
   */
  typename std::map<ExecFlagType, MooseObjectWarehouse<T>>::const_iterator begin() const
  {
    return _execute_objects.begin();
  }
  typename std::map<ExecFlagType, MooseObjectWarehouse<T>>::const_iterator end() const
  {
    return _execute_objects.end();
  }
  ///@}

  /**
   * Updates the active objects storage.
   */
  void updateActive(THREAD_ID tid = 0) override;

  ///@{
  /**
   * Convenience methods for calling object setup methods.
   *
   * Limits call to these methods only to objects being executed on linear/nonlinear iterations.
   */
  void jacobianSetup(THREAD_ID tid = 0) const override;
  void residualSetup(THREAD_ID tid = 0) const override;
  void setup(const ExecFlagType & exec_flag, THREAD_ID tid = 0) const;
  ///@}

  /**
   * Performs a sort using the DependencyResolver.
   * @param tid The thread id to access.
   */
  void sort(THREAD_ID tid = 0);

  bool hasExecType(const ExecFlagType & exec_flag) { return _execute_objects.count(exec_flag) > 0; }

protected:
  // Map of execute objects to storage containers for MooseObjects
  std::map<ExecFlagType, MooseObjectWarehouse<T>> _execute_objects;

  /// A helper method for extracting objects from the various storage containers
  typename std::map<ExecFlagType, MooseObjectWarehouse<T>>::iterator
  getStorageHelper(std::map<ExecFlagType, MooseObjectWarehouse<T>> & objects,
                   ExecFlagType exec_flag) const;
};

template <typename T>
ExecuteMooseObjectWarehouse<T>::ExecuteMooseObjectWarehouse(const ExecFlagEnum & flags,
                                                            bool threaded)
  : MooseObjectWarehouse<T>(threaded)
{
  // Initialize the active/all data structures with the correct map entries and empty vectors
  for (const auto & flag : flags.items())
    _execute_objects.insert(std::make_pair(flag, MooseObjectWarehouse<T>(threaded)));
}

template <typename T>
ExecuteMooseObjectWarehouse<T>::~ExecuteMooseObjectWarehouse()
{
}

template <typename T>
const MooseObjectWarehouse<T> &
ExecuteMooseObjectWarehouse<T>::operator[](ExecFlagType exec_flag) const
{
  // Use find to avoid accidental insertion
  const auto iter = _execute_objects.find(exec_flag);

  if (iter == _execute_objects.end())
    mooseError("Unable to locate the desired execute flag (",
               exec_flag,
               "), the supplied execution flag was likely "
               "not registered.");

  return iter->second;
}

template <typename T>
MooseObjectWarehouse<T> &
ExecuteMooseObjectWarehouse<T>::operator[](ExecFlagType exec_flag)
{
  // Use find to avoid accidental insertion
  const auto iter = _execute_objects.find(exec_flag);

  if (iter == _execute_objects.end())
    mooseError("Unable to locate the desired execute flag (",
               exec_flag,
               "), the supplied execution flag was likely "
               "not registered.");

  return iter->second;
}

template <typename T>
void
ExecuteMooseObjectWarehouse<T>::updateActive(THREAD_ID tid /* = 0 */)
{
  // Update all objects active list
  MooseObjectWarehouse<T>::updateActive(tid);

  // Update the execute flag lists of objects
  for (auto & object_pair : _execute_objects)
    object_pair.second.updateActive(tid);
}

template <typename T>
void
ExecuteMooseObjectWarehouse<T>::jacobianSetup(THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  const auto iter = _execute_objects.find(EXEC_NONLINEAR);
  if (iter != _execute_objects.end())
    iter->second.jacobianSetup(tid);
}

template <typename T>
void
ExecuteMooseObjectWarehouse<T>::residualSetup(THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  const auto iter = _execute_objects.find(EXEC_LINEAR);
  if (iter != _execute_objects.end())
    iter->second.residualSetup(tid);
}

template <typename T>
void
ExecuteMooseObjectWarehouse<T>::setup(const ExecFlagType & exec_flag, THREAD_ID tid /* = 0*/) const
{
  checkThreadID(tid);
  if (exec_flag == EXEC_INITIAL)
    initialSetup(tid);
  else if (exec_flag == EXEC_TIMESTEP_BEGIN)
    timestepSetup(tid);
  else if (exec_flag == EXEC_SUBDOMAIN)
    subdomainSetup(tid);
  else if (exec_flag == EXEC_NONLINEAR)
    jacobianSetup(tid);
  else if (exec_flag == EXEC_LINEAR)
    residualSetup(tid);
}

template <typename T>
void
ExecuteMooseObjectWarehouse<T>::addObject(std::shared_ptr<T> object,
                                          THREAD_ID tid,
                                          bool /*recurse*/)
{
  // Update list of all objects
  MooseObjectWarehouse<T>::addObject(object, tid);

  // Update the execute flag lists of objects
  if (const auto ptr = std::dynamic_pointer_cast<SetupInterface>(object))
    for (const auto & flag : ptr->getExecuteOnEnum())
      _execute_objects[flag].addObject(object, tid);
  else
    mooseError("The object being added (",
               object->name(),
               ") must inherit from SetupInterface to be added to the ExecuteMooseObjectWarehouse "
               "container.");
}

template <typename T>
void
ExecuteMooseObjectWarehouse<T>::sort(THREAD_ID tid /* = 0*/)
{
  // Sort execute object storage
  for (auto & object_pair : _execute_objects)
    object_pair.second.sort(tid);
}
