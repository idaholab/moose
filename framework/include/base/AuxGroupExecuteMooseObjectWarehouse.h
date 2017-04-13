
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

#ifndef AUXGROUPEXECUTEMOOSEOBJECTWAREHOUSEBASE_H
#define AUXGROUPEXECUTEMOOSEOBJECTWAREHOUSEBASE_H

// MOOSE includes
#include "ExecuteMooseObjectWarehouse.h"

class UserObject;

/**
 * General warehouse for storing MooseObjects based on relation to AuxKernel execution.
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
  AuxGroupExecuteMooseObjectWarehouse(bool thread = true);

  /**
   * Access the AuxGroup via bracket operator.
   */
  const ExecuteMooseObjectWarehouse<T> & operator[](Moose::AuxGroup group) const;

  /**
   * Call this inorder to separate the stored objects into the various AuxGroup catagories.
   *
   * @see FEProblemBase::initialSetup()
   */
  void updateDependObjects(const std::set<std::string> & depend_uo, THREAD_ID tid = 0);

  /**
   * Performs a sort using the DependencyResolver.
   */
  void sort(THREAD_ID tid = 0);

  /**
   * Updates the various active lists of objects.
   */
  virtual void updateActive(THREAD_ID tid = 0) override;

protected:
  /// Storage for the PRE_AUX and POST_AUX group sorted objects (ALL is stored in the base class)
  std::vector<ExecuteMooseObjectWarehouse<T>> _group_objects;
};

template <typename T>
AuxGroupExecuteMooseObjectWarehouse<T>::AuxGroupExecuteMooseObjectWarehouse(bool threaded)
  : ExecuteMooseObjectWarehouse<T>(threaded), _group_objects(2) // initialize Pre/Post aux storage
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
AuxGroupExecuteMooseObjectWarehouse<T>::updateDependObjects(const std::set<std::string> & depend_uo,
                                                            THREAD_ID tid)
{
  checkThreadID(tid);

  for (const auto & object_ptr : _all_objects[tid])
  {
    if (depend_uo.find(object_ptr->name()) != depend_uo.end())
      _group_objects[Moose::PRE_AUX].addObject(object_ptr, tid);
    else
      _group_objects[Moose::POST_AUX].addObject(object_ptr, tid);
  }
}

template <typename T>
void
AuxGroupExecuteMooseObjectWarehouse<T>::sort(THREAD_ID tid /*= 0*/)
{
  ExecuteMooseObjectWarehouse<T>::sort(tid);
  _group_objects[Moose::PRE_AUX].sort(tid);
  _group_objects[Moose::POST_AUX].sort(tid);
}

template <typename T>
void
AuxGroupExecuteMooseObjectWarehouse<T>::updateActive(THREAD_ID tid /*=0*/)
{
  ExecuteMooseObjectWarehouse<T>::updateActive(tid);
  _group_objects[Moose::PRE_AUX].updateActive(tid);
  _group_objects[Moose::POST_AUX].updateActive(tid);
}

#endif // AUXGROUPEXECUTEMOOSEOBJECTWAREHOUSEBASE_H
