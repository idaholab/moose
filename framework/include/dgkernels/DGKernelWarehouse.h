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

#include "Moose.h"

//libMesh
#include "libmesh/libmesh_common.h"

#include <vector>
#include <map>
#include <set>

class DGKernel;

/**
 * Holds kernels and provides some services
 */
class DGKernelWarehouse
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
   * Get list of all kernels
   * @return The list of all active kernels
   */
  const std::vector<DGKernel *> & all() { return _all_dg_kernels; }

  /**
   * Get the list of all active kernels
   * @return The list of all active kernels
   */
  const std::vector<DGKernel *> & active() { return _active_dg_kernels; }

  void addDGKernel(DGKernel *dg_kernel);

  void updateActiveDGKernels(Real t, Real dt);

protected:
  std::vector<DGKernel *> _active_dg_kernels;
  std::vector<DGKernel *> _all_dg_kernels;
};

#endif // DGKERNELWAREHOUSE_H
