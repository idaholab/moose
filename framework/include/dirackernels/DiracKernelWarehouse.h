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

#ifndef DIRACKERNELWAREHOUSE_H
#define DIRACKERNELWAREHOUSE_H

#include "Warehouse.h"

#include <vector>

class DiracKernel;

/**
 * Holds DiracKernels and provides some services
 */
class DiracKernelWarehouse : public Warehouse<DiracKernel>
{
public:
  DiracKernelWarehouse();
  virtual ~DiracKernelWarehouse();

  // Setup /////
  void initialSetup();
  void timestepSetup();
  void residualSetup();
  void jacobianSetup();

  /**
   * Adds a Dirac kernel
   * @param kernel The DiracKernel being added
   */
  void addDiracKernel(MooseSharedPointer<DiracKernel> & kernel);

protected:
  /**
   * We are using MooseSharedPointer to handle the cleanup of the pointers at the end of execution.
   * This is necessary since several warehouses might be sharing a single instance of a MooseObject.
   */
  std::vector<MooseSharedPointer<DiracKernel> > _all_ptrs;
};

#endif // DIRACKERNELWAREHOUSE_H
