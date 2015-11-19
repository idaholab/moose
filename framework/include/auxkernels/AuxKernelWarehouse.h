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

#ifndef AUXKERNELWAREHOUSE_H
#define AUXKERNELWAREHOUSE_H

// MOOSE includes
#include "WarehouseBase.h"
#include "AuxKernel.h"

class AuxKernelWarehouse : public WarehouseBase<AuxKernel>
{
public:

  /// Enum used of retieving the different storage containers in this warehouse
  enum AuxKernelType{NODAL, ELEMENTAL, ALL};

  /**
   * Constructor.
   */
  AuxKernelWarehouse();

  /**
   * Destructor.
   */
  ~AuxKernelWarehouse(){}

  /**
   * Adds AuxKernel object to the correct storage structure (nodal or elemental)
   */
  virtual void addObject(MooseSharedPointer<AuxKernel> object, THREAD_ID tid = 0);

  /**
   * Return the storage strcuture for type of AuxKernel and execute flag.
   * @param kernel_type The type of AuxKernel objects to return.
   * @param exec_flag The execution flag desired.
   *
   * To get all the AuxKernels use:
   *   getStorage(AuxKernelWarehouse::ALL);
   */
  const MooseObjectStorage<AuxKernel> & getStorage(AuxKernelType kernel_type, ExecFlagType exec_type = EXEC_NONE);

  /**
   * Method to determine if active objects exist on a boundary.
   */
  bool hasActiveBoundaryObjects(BoundaryID id, THREAD_ID tid = 0);

  /**
   * Sorts the AuxScalarKenels prior to calling the initialSetup.
   */
  virtual void initialSetup(THREAD_ID tid);

  ///@{
  /**
   * Call setup methods for both elemental and nodal AuxKernel objects
   */
  virtual void timestepSetup(THREAD_ID tid);
  virtual void subdomainSetup(THREAD_ID tid);
  virtual void jacobianSetup(THREAD_ID tid);
  virtual void residualSetup(THREAD_ID tid);
  ///@}

  /**
   * Update the active status of elemental and nodal AuxKernel objects
   */
  virtual void updateActive(THREAD_ID tid);

private:

  /// Storage for all AuxKernel objects, needed for calling setup methods
  MooseObjectStorage<AuxKernel> _all;

  /// Storage for elemental AuxKernel objects
  ExecuteMooseObjectStorage<AuxKernel> _elemental;

  /// Storage for nodal AuxKernel objects
  ExecuteMooseObjectStorage<AuxKernel> _nodal;

};

#endif // AUXSCALARKERNELWAREHOUSE_H
