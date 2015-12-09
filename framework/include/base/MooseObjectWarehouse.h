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
#include "MooseObjectStorage.h"

/**
 * A general warehouse for storing a single set of objects.
 */
template <typename T>
class MooseObjectWarehouse
{
public:

  /**
   * Constructor.
   */
  MooseObjectWarehouse(bool threaded = true);

  /**
   * Destructor.
   */
  virtual ~MooseObjectWarehouse(){}

  /**
   * Add the object to the stroage containers for the given thread.
   * @param tid The thread id on which the object shall be stored.
   */
  virtual void addObject(MooseSharedPointer<T> object, THREAD_ID tid = 0);

  ///@{
  /**
   * Access to the objects in this warehouse.
   */
  virtual MooseObjectStorage<T> & getStorage() { return _all_objects; }
  virtual const MooseObjectStorage<T> & getStorage() const { return _all_objects; }
  ///@}

  /**
   * Return the number of objects stored for the given thread.
   */
  unsigned int size(THREAD_ID tid = 0);

  ///@{
  /**
   * Convienence methods for calling setup methods
   */
  virtual void initialSetup(THREAD_ID tid = 0);
  virtual void timestepSetup(THREAD_ID tid = 0);
  virtual void subdomainSetup(THREAD_ID tid = 0);
  virtual void jacobianSetup(THREAD_ID tid = 0);
  virtual void residualSetup(THREAD_ID tid = 0);
  virtual void updateActive(THREAD_ID tid = 0);
  ///@}

protected:

  /// Storage for all objects
  MooseObjectStorage<T> _all_objects;

};


template<typename T>
MooseObjectWarehouse<T>::MooseObjectWarehouse(bool threaded /*=true*/) :
    _all_objects(threaded)
{
}


template<typename T>
void
MooseObjectWarehouse<T>::addObject(MooseSharedPointer<T> object, THREAD_ID tid)
{
  _all_objects.addObject(object, tid);
}

template<typename T>
unsigned int
MooseObjectWarehouse<T>::size(THREAD_ID tid)
{
  return _all_objects.size(tid);
}


template<typename T>
void
MooseObjectWarehouse<T>::initialSetup(THREAD_ID tid)
{
  _all_objects.initialSetup(tid);
}


template<typename T>
void
MooseObjectWarehouse<T>::timestepSetup(THREAD_ID tid)
{
  _all_objects.timestepSetup(tid);
}


template<typename T>
void
MooseObjectWarehouse<T>::subdomainSetup(THREAD_ID tid)
{
  _all_objects.subdomainSetup(tid);
}


template<typename T>
void
MooseObjectWarehouse<T>::jacobianSetup(THREAD_ID tid)
{
  _all_objects.jacobianSetup(tid);
}


template<typename T>
void
MooseObjectWarehouse<T>::residualSetup(THREAD_ID tid)
{
  _all_objects.residualSetup(tid);
}


template<typename T>
void
MooseObjectWarehouse<T>::updateActive(THREAD_ID tid)
{
  _all_objects.updateActive(tid);
}

#endif // MOOSEOBJECTWAREHOUSE_H
