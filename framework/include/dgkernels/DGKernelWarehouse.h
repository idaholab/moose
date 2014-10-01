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

#ifndef DGKERNELWAREHOUSE_H
#define DGKERNELWAREHOUSE_H

#include "Warehouse.h"

#include <vector>

class DGKernel;

/**
 * Holds kernels and provides some services
 */
class DGKernelWarehouse : public Warehouse<DGKernel>
{
public:
  DGKernelWarehouse();
  virtual ~DGKernelWarehouse();

  // Setup /////
  void initialSetup();
  void timestepSetup();
  void residualSetup();
  void jacobianSetup();

  /**
   * Get the list of all active kernels
   * @return The list of all active kernels
   */
  const std::vector<DGKernel *> & active() const { return _active_dg_kernels; }

  void addDGKernel(MooseSharedPointer<DGKernel> & dg_kernel);

  void updateActiveDGKernels(Real t, Real dt);

protected:
  /**
   * We are using MooseSharedPointer to handle the cleanup of the pointers at the end of execution.
   * This is necessary since several warehouses might be sharing a single instance of a MooseObject.
   */
  std::vector<MooseSharedPointer<DGKernel> > _all_ptrs;

  std::vector<DGKernel *> _active_dg_kernels;
};

#endif // DGKERNELWAREHOUSE_H
