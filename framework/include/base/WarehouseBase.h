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

#ifndef WAREHOUSEBASE_H
#define WAREHOUSEBASE_H

// MOOSE includes
#include "MooseError.h"
#include "MooseTypes.h"

/**
 * An abstract warehouse class.
 */
template<typename BaseType>
class WarehouseBase
{
public:

  /**
   * Constructor.
   * @param threaded When true threaded storage is enabled.
   */
  WarehouseBase(bool threaded = true);

  /**
   * Desctructor.
   */
  virtual ~WarehouseBase();

  /**
   * Add an object that is a type of the base warehouse object.
   * @param object A shared pointer to the object being added to the warehouse.
   * @param tid The thread on which to operate.
   */
  virtual void addObject(MooseSharedPointer<BaseType> object, THREAD_ID tid = 0) = 0;

  ///@{
  /**
   * Convienence methods for calling setup methods of objects stored in the warhouse.
   */
  virtual void initialSetup(THREAD_ID tid = 0) = 0;
  virtual void timestepSetup(THREAD_ID tid = 0) = 0;
  virtual void subdomainSetup(THREAD_ID tid = 0) = 0;
  virtual void jacobianSetup(THREAD_ID tid = 0) = 0;
  virtual void residualSetup(THREAD_ID tid = 0) = 0;
  ///@}

  /**
   * Updates the active objects stored in the warehouse.
   */
  virtual void updateActive(THREAD_ID tid = 0)= 0;

protected:

  /// The number of threaded copies (1 if the warehouse is not threaded)
  THREAD_ID _num_threads;

};


template<typename BaseType>
WarehouseBase<BaseType>::WarehouseBase(bool threaded) :
    _num_threads(threaded ? libMesh::n_threads() : 1)//,
{
}


template<typename BaseType>
WarehouseBase<BaseType>::~WarehouseBase()
{
}

#endif // WAREHOUSEBASE_H
