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

#ifndef EXECUTEMOOSEOBJECTWAREHOUSE_H
#define EXECUTEMOOSEOBJECTWAREHOUSE_H

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
  ExecuteMooseObjectWarehouse(bool threaded = true);

  virtual ~ExecuteMooseObjectWarehouse();

  /**
   * Adds an object to the storage structure.
   * @param object A shared pointer to the object being added
   */
  virtual void addObject(std::shared_ptr<T> object, THREAD_ID tid = 0);

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
  virtual void updateActive(THREAD_ID tid = 0);

  ///@{
  /**
   * Convenience methods for calling object setup methods.
   *
   * Limits call to these methods only to objects being executed on linear/nonlinear iterations.
   */
  void jacobianSetup(THREAD_ID tid = 0) const;
  void residualSetup(THREAD_ID tid = 0) const;
  void setup(const ExecFlagType & exec_flag, THREAD_ID tid = 0) const;
  ///@}

  /**
   * Performs a sort using the DependencyResolver.
   * @param tid The thread id to access.
   */
  void sort(THREAD_ID tid = 0);

protected:
  // Map of execute objects to storage containers for MooseObjects
  std::map<ExecFlagType, MooseObjectWarehouse<T>> _execute_objects;

  /// A helper method for extracting objects from the various storage containers
  typename std::map<ExecFlagType, MooseObjectWarehouse<T>>::iterator
  getStorageHelper(std::map<ExecFlagType, MooseObjectWarehouse<T>> & objects,
                   ExecFlagType exec_flag) const;
};

template <typename T>
ExecuteMooseObjectWarehouse<T>::ExecuteMooseObjectWarehouse(bool threaded)
  : MooseObjectWarehouse<T>(threaded)
{
  // Initialize the active/all data structures with the correct map entries and empty vectors
  for (const auto & flag : Moose::execute_flags)
    _execute_objects.insert(std::make_pair(flag.first, MooseObjectWarehouse<T>(threaded)));
}

template <typename T>
ExecuteMooseObjectWarehouse<T>::~ExecuteMooseObjectWarehouse()
{
}

template <typename T>
const MooseObjectWarehouse<T> & ExecuteMooseObjectWarehouse<T>::
operator[](ExecFlagType exec_flag) const
{
  // Use find to avoid accidental insertion
  const auto iter = _execute_objects.find(exec_flag);

  if (iter == _execute_objects.end())
    mooseError("Unable to locate the desired execute flag, the supplied execution flag was likely "
               "not registered.");

  return iter->second;
}

template <typename T>
MooseObjectWarehouse<T> & ExecuteMooseObjectWarehouse<T>::operator[](ExecFlagType exec_flag)
{
  // Use find to avoid accidental insertion
  const auto iter = _execute_objects.find(exec_flag);

  if (iter == _execute_objects.end())
    mooseError("Unable to locate the desired execute flag, the supplied execution flag was likely "
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
ExecuteMooseObjectWarehouse<T>::addObject(std::shared_ptr<T> object, THREAD_ID tid /*=0*/)
{
  // Update list of all objects
  MooseObjectWarehouse<T>::addObject(object, tid);

  // Update the execute flag lists of objects
  std::shared_ptr<SetupInterface> ptr = std::dynamic_pointer_cast<SetupInterface>(object);
  if (ptr)
  {
    for (const ExecFlagType & flag : ptr->getExecuteOnEnum().getCurrentIDs())
      _execute_objects[flag].addObject(object, tid);
  }
  else
    mooseError("The object being added (",
               object->name(),
               ") must inherit from SetupInterface to "
               "be added to the "
               "ExecuteMooseObjectWarehouse "
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

#endif // EXECUTEMOOSEOBJECTWAREHOUSE_H
