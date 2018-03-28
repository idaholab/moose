//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef AUXGROUPEXECUTEMOOSEOBJECTWAREHOUSEBASE_H
#define AUXGROUPEXECUTEMOOSEOBJECTWAREHOUSEBASE_H

// MOOSE includes
#include "ExecuteMooseObjectWarehouse.h"

class UserObject;

/**
 * General warehouse for storing MooseObjects based on relation to IC and AuxKernel execution.
 */
template <typename T>
class AuxGroupExecuteMooseObjectWarehouse : public ExecuteMooseObjectWarehouse<T>
{

public:
  /// Using these from base class
  using MooseObjectWarehouse<T>::checkThreadID;
  using ExecuteMooseObjectWarehouse<T>::_all_objects;
  using ExecuteMooseObjectWarehouse<T>::_execute_objects;
  using ExecuteMooseObjectWarehouse<T>::_num_threads;

  /**
   * Constructor.
   */
  AuxGroupExecuteMooseObjectWarehouse(const ExecFlagEnum & flags, bool thread = true);

  /**
   * Access the AuxGroup via bracket operator.
   */
  const ExecuteMooseObjectWarehouse<T> & operator[](Moose::AuxGroup group) const;

  /**
   * Call this to separate the stored objects into the various AuxGroup categories.
   *
   * @see FEProblemBase::initialSetup()
   */
  void updateDependObjects(const std::set<std::string> & depend_ic,
                           const std::set<std::string> & depend_aux,
                           THREAD_ID tid = 0);

  /**
   * Performs a sort using the DependencyResolver.
   */
  void sort(THREAD_ID tid = 0);

  /**
   * Updates the various active lists of objects.
   */
  virtual void updateActive(THREAD_ID tid = 0) override;

protected:
  /// Storage for the group sorted objects (ALL is stored in the base class)
  std::vector<ExecuteMooseObjectWarehouse<T>> _group_objects;
};

template <typename T>
AuxGroupExecuteMooseObjectWarehouse<T>::AuxGroupExecuteMooseObjectWarehouse(
    const ExecFlagEnum & flags, bool threaded)
  : ExecuteMooseObjectWarehouse<T>(flags, threaded),
    _group_objects(3, ExecuteMooseObjectWarehouse<T>(flags, threaded)) // initialize group storage
{
}

template <typename T>
const ExecuteMooseObjectWarehouse<T> & AuxGroupExecuteMooseObjectWarehouse<T>::
operator[](Moose::AuxGroup group) const
{
  if (group == Moose::ALL)
    return *this;
  return _group_objects[group];
}

template <typename T>
void
AuxGroupExecuteMooseObjectWarehouse<T>::updateDependObjects(
    const std::set<std::string> & depend_ic,
    const std::set<std::string> & depend_aux,
    THREAD_ID tid)
{
  checkThreadID(tid);

  const std::uint16_t initial_flag_mask = static_cast<std::uint16_t>(EXEC_INITIAL);
  const std::uint16_t not_initial_flag_mask = ~static_cast<std::uint16_t>(EXEC_INITIAL);
  const std::uint16_t all_flags = std::numeric_limits<std::uint16_t>::max();

  for (const auto & object_ptr : _all_objects[tid])
  {
    bool already_added = false;
    if (depend_ic.find(object_ptr->name()) != depend_ic.end())
    {
      _group_objects[Moose::PRE_IC].addObjectMask(object_ptr, tid, initial_flag_mask);
      already_added = !object_ptr->shouldDuplicateInitialExecution();
    }

    std::uint16_t remaining_flags = already_added ? not_initial_flag_mask : all_flags;
    if ((object_ptr->isParamValid("force_preaux") &&
         object_ptr->template getParam<bool>("force_preaux")) ||
        depend_aux.find(object_ptr->name()) != depend_aux.end() ||
        depend_ic.find(object_ptr->name()) != depend_ic.end())
      _group_objects[Moose::PRE_AUX].addObjectMask(object_ptr, tid, remaining_flags);
    else
      _group_objects[Moose::POST_AUX].addObjectMask(object_ptr, tid, remaining_flags);
  }
}

template <typename T>
void
AuxGroupExecuteMooseObjectWarehouse<T>::sort(THREAD_ID tid /*= 0*/)
{
  ExecuteMooseObjectWarehouse<T>::sort(tid);
  _group_objects[Moose::PRE_IC].sort(tid);
  _group_objects[Moose::PRE_AUX].sort(tid);
  _group_objects[Moose::POST_AUX].sort(tid);
}

template <typename T>
void
AuxGroupExecuteMooseObjectWarehouse<T>::updateActive(THREAD_ID tid /*=0*/)
{
  ExecuteMooseObjectWarehouse<T>::updateActive(tid);
  _group_objects[Moose::PRE_IC].updateActive(tid);
  _group_objects[Moose::PRE_AUX].updateActive(tid);
  _group_objects[Moose::POST_AUX].updateActive(tid);
}

#endif // AUXGROUPEXECUTEMOOSEOBJECTWAREHOUSEBASE_H
