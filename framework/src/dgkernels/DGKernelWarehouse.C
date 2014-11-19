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
#include "DGKernel.h"

DGKernelWarehouse::DGKernelWarehouse() :
    Warehouse<DGKernel>()
{
}

DGKernelWarehouse::~DGKernelWarehouse()
{
}

void
DGKernelWarehouse::initialSetup()
{
  for (std::vector<DGKernel *>::const_iterator i = all().begin(); i != all().end(); ++i)
    (*i)->initialSetup();
}

void
DGKernelWarehouse::timestepSetup()
{
  for (std::vector<DGKernel *>::const_iterator i = all().begin(); i != all().end(); ++i)
    (*i)->timestepSetup();
}

void
DGKernelWarehouse::residualSetup()
{
  for (std::vector<DGKernel *>::const_iterator i = all().begin(); i != all().end(); ++i)
    (*i)->residualSetup();
}

void
DGKernelWarehouse::jacobianSetup()
{
  for (std::vector<DGKernel *>::const_iterator i = all().begin(); i != all().end(); ++i)
    (*i)->jacobianSetup();
}

void
DGKernelWarehouse::addDGKernel(MooseSharedPointer<DGKernel> & dg_kernel)
{
  _all_ptrs.push_back(dg_kernel);
  _all_objects.push_back(dg_kernel.get());
}

void
DGKernelWarehouse::updateActiveDGKernels(Real /*t*/, Real /*dt*/)
{
  _active_dg_kernels.clear();

  // add kernels that live everywhere
  for (std::vector<DGKernel *>::const_iterator it = _all_objects.begin(); it != _all_objects.end(); ++it)
  {
    DGKernel * dg_kernel = *it;
    // FIXME: add startTime/stopTime to DGKernel
//    if (dg_kernel->startTime() <= t + (1e-6 * dt) && dg_kernel->stopTime() >= t + (1e-6 * dt))
      _active_dg_kernels.push_back(dg_kernel);
  }
}
