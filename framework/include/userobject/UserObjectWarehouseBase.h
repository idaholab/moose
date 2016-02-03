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

#ifndef USEROBJECTWAREHOUSEBASE_H
#define USEROBJECTWAREHOUSEBASE_H

// MOOSE includes
#include "ExecuteMooseObjectWarehouse.h"
#include "UserObjectWarehouse.h" // for GROUP, needs to be moved
#include "Postprocessor.h"

class UserObject;

/**
 * General warehouse for storing UserObjects
 */

template<typename T>
class UserObjectWarehouseBase : public ExecuteMooseObjectWarehouse<T>
{

public:

  using MooseObjectWarehouse<T>::checkThreadID;
  using ExecuteMooseObjectWarehouse<T>::_all_objects;
  using ExecuteMooseObjectWarehouse<T>::_num_threads;


  //enum GROUP{ ALL, PRE_AUX, POST_AUX };



  UserObjectWarehouseBase(bool thread = true);

  virtual void addObject(MooseSharedPointer<T> object, THREAD_ID tid = 0);

  const ExecuteMooseObjectWarehouse<T> & operator[](UserObjectWarehouse::GROUP group) const;

  void initialSetup(const std::set<std::string> & depend_uo, THREAD_ID tid = 0);

  //MooseSharedPointer<Postprocessor> getPostprocessor(MooseSharedPointer<T> object, THREAD_ID tid = 0) const;

  /**
   * Performs a sort using the DependencyResolver.
   */
  void sort(THREAD_ID tid = 0);

protected:

  std::map<UserObjectWarehouse::GROUP, ExecuteMooseObjectWarehouse<T> > _group_objects;

  std::vector<std::map<MooseSharedPointer<T>, MooseSharedPointer<Postprocessor> > > _postprocessors;
};


template<typename T>
UserObjectWarehouseBase<T>::UserObjectWarehouseBase(bool threaded) :
    ExecuteMooseObjectWarehouse<T>(threaded),
    _postprocessors(_num_threads)
{
  // Initialize Pre/Post aux storage
  //_group_storage[ALL] = ExecuteMooseObjectWarehouse<T>(threaded);
  _group_objects.insert(std::pair<UserObjectWarehouse::GROUP, ExecuteMooseObjectWarehouse<T> >(UserObjectWarehouse::PRE_AUX, ExecuteMooseObjectWarehouse<T>(threaded)));
  _group_objects.insert(std::pair<UserObjectWarehouse::GROUP, ExecuteMooseObjectWarehouse<T> >(UserObjectWarehouse::POST_AUX, ExecuteMooseObjectWarehouse<T>(threaded)));
  //_active_group_storage[ALL] = ExecuteMooseObjectWarehouse<T>(threaded);
//  _active_group_storage[PRE_AUX] = ExecuteMooseObjectWarehouse<T>(threaded);
//  _active_group_storage[POST_AUX] = ExecuteMooseObjectWarehouse<T>(threaded);
}


template<typename T>
const ExecuteMooseObjectWarehouse<T> &
UserObjectWarehouseBase<T>::operator[](UserObjectWarehouse::GROUP group) const
{
  typename std::map<UserObjectWarehouse::GROUP, ExecuteMooseObjectWarehouse<T> >::const_iterator iter = _group_objects.find(group);
  if (iter == _group_objects.end())
    mooseError("Unable to locate the desired group flag, objects only exists for PRE_AUX and POST_AUX groups.");
  return iter->second;
}


template<typename T>
void
UserObjectWarehouseBase<T>::addObject(MooseSharedPointer<T> object, THREAD_ID tid)
{
  ExecuteMooseObjectWarehouse<T>::addObject(object, tid);

  MooseSharedPointer<Postprocessor> pp = MooseSharedNamespace::dynamic_pointer_cast<Postprocessor>(object);
  if (pp)
    _postprocessors[tid][object] = pp;
}

/*
template<typename T>
MooseSharedPointer<Postprocessor>
UserObjectWarehouseBase<T>::getPostprocessor(MooseSharedPointer<T> object, THREAD_ID tid) const
{
  checkThreadID(tid);
  typename std::map<MooseSharedPointer<T>, MooseSharedPointer<Postprocessor> >::const_iterator iter = _postprocessors[tid].find(object);
  if (iter != _postprocessors[tid].end())
    return (iter->second);

  MooseSharedPointer<Postprocessor> pp;
  return pp;

}
*/


template<typename T>
void
UserObjectWarehouseBase<T>::initialSetup(const std::set<std::string> & depend_uo, THREAD_ID tid)
{
  checkThreadID(tid);

  ExecuteMooseObjectWarehouse<T>::initialSetup(tid);

  for (typename std::vector<MooseSharedPointer<T> >::const_iterator it = _all_objects[tid].begin(); it != _all_objects[tid].end(); ++it)
  {
    if (depend_uo.find((*it)->name()) != depend_uo.end())
      _group_objects[UserObjectWarehouse::PRE_AUX].addObject((*it));
    else
      _group_objects[UserObjectWarehouse::POST_AUX].addObject((*it));
  }
}


template<typename T>
void
UserObjectWarehouseBase<T>::sort(THREAD_ID tid/*= 0*/)
{
  ExecuteMooseObjectWarehouse<T>::sort(tid);
  _group_objects[UserObjectWarehouse::PRE_AUX].sort(tid);
  _group_objects[UserObjectWarehouse::POST_AUX].sort(tid);
}


#endif // USEROBJECTWAREHOUSEBASE_H
