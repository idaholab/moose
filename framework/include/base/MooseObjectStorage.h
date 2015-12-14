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

#ifndef MOOSEOBJECTSTORAGE_H
#define MOOSEOBJECTSTORAGE_H

// MOOSE includes
#include "MooseObjectStorageBase.h"

/**
 * A storage container for MooseObjects.
 *
 * This container is stores threaded copies and includes automatic storage of block/boundary
 * restricted objects. It also, maintains lists of active objects for use by Controls.
 */
template<typename T>
class MooseObjectStorage : public MooseObjectStorageBase<T>
{
public:

  using MooseObjectStorageBase<T>::checkThreadID;
  using MooseObjectStorageBase<T>::_active_objects;

  /**
   * Constructor.
   * @param threaded When true (default) threaded storage is enabled.
   */
  MooseObjectStorage(bool threaded = true);

  ///@{
  /**
   * Convenience methods for calling object setup methods.
   */
  virtual void initialSetup(THREAD_ID tid = 0) const;
  virtual void timestepSetup(THREAD_ID tid = 0) const;
  virtual void subdomainSetup(THREAD_ID tid = 0) const;
  virtual void jacobianSetup(THREAD_ID tid = 0) const;
  virtual void residualSetup(THREAD_ID tid = 0) const;
  ///@}
};


template<typename T>
MooseObjectStorage<T>::MooseObjectStorage(bool threaded /*=true*/) :
    MooseObjectStorageBase<T>(threaded)
{
}

template<typename T>
void
MooseObjectStorage<T>::initialSetup(THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  typename std::vector<MooseSharedPointer<T> >::const_iterator it;
  for (it = _active_objects[tid].begin(); it != _active_objects[tid].end(); ++it)
    (*it)->initialSetup();
}


template<typename T>
void
MooseObjectStorage<T>::timestepSetup(THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  typename std::vector<MooseSharedPointer<T> >::const_iterator it;
  for (it = _active_objects[tid].begin(); it != _active_objects[tid].end(); ++it)
    (*it)->timestepSetup();
}


template<typename T>
void
MooseObjectStorage<T>::subdomainSetup(THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  typename std::vector<MooseSharedPointer<T> >::const_iterator it;
  for (it = _active_objects[tid].begin(); it != _active_objects[tid].end(); ++it)
    (*it)->subdomainSetup();
}


template<typename T>
void
MooseObjectStorage<T>::jacobianSetup(THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  typename std::vector<MooseSharedPointer<T> >::const_iterator it;
  for (it = _active_objects[tid].begin(); it != _active_objects[tid].end(); ++it)
    (*it)->jacobianSetup();
}


template<typename T>
void
MooseObjectStorage<T>::residualSetup(THREAD_ID tid/* = 0*/) const
{
  checkThreadID(tid);
  typename std::vector<MooseSharedPointer<T> >::const_iterator it;
  for (it = _active_objects[tid].begin(); it != _active_objects[tid].end(); ++it)
    (*it)->residualSetup();
}

#endif // MOOSEOBJECTSTORAGE_H
