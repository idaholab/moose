
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
#include "Postprocessor.h"

class UserObject;

/**
 * General warehouse for storing MooseObjects based on relation to AuxKernel execution.
 */
template<typename T>
class AuxGroupExecuteMooseObjectWarehouse : public ExecuteMooseObjectWarehouse<T>
{

public:

  /// Using these from base class
  using MooseObjectWarehouse<T>::checkThreadID;
  using ExecuteMooseObjectWarehouse<T>::_all_objects;
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
   * @see FEProblem::initialSetup()
   */
  void updateDependObjects(const std::set<std::string> & depend_uo, THREAD_ID tid = 0);

  /**
   * Performs a sort using the DependencyResolver.
   */
  void sort(THREAD_ID tid = 0);

protected:

  /// Storage for the group sorted objects
  std::map<Moose::AuxGroup, ExecuteMooseObjectWarehouse<T> > _group_objects;
};


template<typename T>
AuxGroupExecuteMooseObjectWarehouse<T>::AuxGroupExecuteMooseObjectWarehouse(bool threaded) :
    ExecuteMooseObjectWarehouse<T>(threaded)
{
  // Initialize Pre/Post aux storage
  _group_objects.insert(std::pair<Moose::AuxGroup, ExecuteMooseObjectWarehouse<T> >(Moose::PRE_AUX, ExecuteMooseObjectWarehouse<T>(threaded)));
  _group_objects.insert(std::pair<Moose::AuxGroup, ExecuteMooseObjectWarehouse<T> >(Moose::POST_AUX, ExecuteMooseObjectWarehouse<T>(threaded)));
}


template<typename T>
const ExecuteMooseObjectWarehouse<T> &
AuxGroupExecuteMooseObjectWarehouse<T>::operator[](Moose::AuxGroup group) const
{
  typename std::map<Moose::AuxGroup, ExecuteMooseObjectWarehouse<T> >::const_iterator iter = _group_objects.find(group);
  if (iter == _group_objects.end())
    mooseError("Unable to locate the desired group flag, objects only exists for PRE_AUX and POST_AUX groups.");
  return iter->second;
}


template<typename T>
void
AuxGroupExecuteMooseObjectWarehouse<T>::updateDependObjects(const std::set<std::string> & depend_uo, THREAD_ID tid)
{
  checkThreadID(tid);

  for (typename std::vector<MooseSharedPointer<T> >::const_iterator it = _all_objects[tid].begin(); it != _all_objects[tid].end(); ++it)
  {
    if (depend_uo.find((*it)->name()) != depend_uo.end())
      _group_objects[Moose::PRE_AUX].addObject(*it, tid);
    else
      _group_objects[Moose::POST_AUX].addObject(*it, tid);
  }
}


template<typename T>
void
AuxGroupExecuteMooseObjectWarehouse<T>::sort(THREAD_ID tid/*= 0*/)
{
  ExecuteMooseObjectWarehouse<T>::sort(tid);
  _group_objects[Moose::PRE_AUX].sort(tid);
  _group_objects[Moose::POST_AUX].sort(tid);
}

#endif // AUXGROUPEXECUTEMOOSEOBJECTWAREHOUSEBASE_H
