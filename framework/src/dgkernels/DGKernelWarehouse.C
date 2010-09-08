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

#include "DGKernelWarehouse.h"
#include "MooseSystem.h"

DGKernelWarehouse::DGKernelWarehouse()
{
}

DGKernelWarehouse::~DGKernelWarehouse()
{
  for (DGKernelIterator i=_active_dg_kernels.begin(); i!=_active_dg_kernels.end(); ++i)
    delete *i;
}

void
DGKernelWarehouse::addDGKernel(DGKernel *dg_kernel)
{
  _all_dg_kernels.push_back(dg_kernel);
}

DGKernelIterator
DGKernelWarehouse::activeDGKernelsBegin()
{
  return _active_dg_kernels.begin();
}

DGKernelIterator
DGKernelWarehouse::activeDGKernelsEnd()
{
  return _active_dg_kernels.end();
}

void
DGKernelWarehouse::updateActiveDGKernels(Real t, Real dt)
{
  _active_dg_kernels.clear();

  DGKernelIterator all_it = _all_dg_kernels.begin();
  DGKernelIterator all_end = _all_dg_kernels.end();

  for(; all_it != all_end; ++all_it)
    if((*all_it)->startTime() <= t + (1e-6 * dt) && (*all_it)->stopTime() >= t + (1e-6 * dt))
      _active_dg_kernels.push_back(*all_it);
}
