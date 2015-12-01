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
#include "ExecuteMooseObjectStorage.h"

/**
 * A general warehouse for storing a single set of objects.
 */
template <typename T>
class ExecuteMooseObjectWarehouse : public MooseObjectWarehouse<T>
{
public:

  using MooseObjectWarehouse<T>::_all_objects;

  /**
   * Constructor.
   */
  ExecuteMooseObjectWarehouse(bool threaded = true);

  /**
   * Destructor.
   */
  virtual ~ExecuteMooseObjectWarehouse(){}

  /**
   * Add the object to the stroage containers for the given thread.
   * @param tid The thread id on which the object shall be stored.
   */
  virtual void addObject(MooseSharedPointer<T> object, THREAD_ID tid = 0);

  ///@{
  /**
   * Access to the objects in this warehouse.
   *
   * Note, the base class methods must be repeated here for the overloading to work properly for this
   * templated class.
   */
  virtual MooseObjectStorage<T> & getStorage() { return _all_objects; }
  virtual const MooseObjectStorage<T> & getStorage() const { return _all_objects; }
  virtual MooseObjectStorage<T> & getStorage(ExecFlagType exec_type) { return _execute_objects[exec_type]; }
  virtual const MooseObjectStorage<T> & getStorage(ExecFlagType exec_type) const { return _execute_objects[exec_type]; }
  ///@}

  ///@{
  /**
   * Convienence methods for calling setup methods
   */
  virtual void jacobianSetup(THREAD_ID tid = 0);
  virtual void residualSetup(THREAD_ID tid = 0);
  virtual void updateActive(THREAD_ID tid = 0);
  ///@}

protected:

  /// Storage for objects based on execution flags
  ExecuteMooseObjectStorage<T> _execute_objects;

};


template<typename T>
ExecuteMooseObjectWarehouse<T>::ExecuteMooseObjectWarehouse(bool threaded /*=true*/) :
    MooseObjectWarehouse<T>(threaded),
    _execute_objects(threaded)
{
}


template<typename T>
void
ExecuteMooseObjectWarehouse<T>::addObject(MooseSharedPointer<T> object, THREAD_ID tid)
{
  _all_objects.addObject(object,tid);
  _execute_objects.addObject(object, tid);
}

template<typename T>
void
ExecuteMooseObjectWarehouse<T>::jacobianSetup(THREAD_ID tid)
{
  _execute_objects[EXEC_NONLINEAR].jacobianSetup(tid);
}


template<typename T>
void
ExecuteMooseObjectWarehouse<T>::residualSetup(THREAD_ID tid)
{
  _execute_objects[EXEC_LINEAR].residualSetup(tid);
}


template<typename T>
void
ExecuteMooseObjectWarehouse<T>::updateActive(THREAD_ID tid)
{
  _all_objects.updateActive(tid);
  _execute_objects.updateActive(tid);
}

#endif // EXECUTEMOOSEOBJECTWAREHOUSE_H
