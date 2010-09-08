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

#include <vector>
#include <map>
#include <set>

#include "DGKernel.h"


/**
 * Typedef to hide implementation details
 */
typedef std::vector<DGKernel *>::iterator DGKernelIterator;


/**
 * Holds kernels and provides some services
 */
class DGKernelWarehouse
{
public:
  DGKernelWarehouse();
  virtual ~DGKernelWarehouse();

  DGKernelIterator activeDGKernelsBegin();
  DGKernelIterator activeDGKernelsEnd();

  void addDGKernel(DGKernel *dg_kernel);

  void updateActiveDGKernels(Real t, Real dt);

  std::vector<DGKernel *> _active_dg_kernels;
  std::vector<DGKernel *> _all_dg_kernels;
};

#endif // DGKERNELWAREHOUSE_H
