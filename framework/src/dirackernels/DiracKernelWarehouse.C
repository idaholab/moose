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

#include "DiracKernelWarehouse.h"
#include "DiracKernel.h"


DiracKernelWarehouse::DiracKernelWarehouse()
{
}

DiracKernelWarehouse::~DiracKernelWarehouse()
{
}

void
DiracKernelWarehouse::initialSetup()
{
  for (unsigned int i=0; i<_dirac_kernels.size(); i++)
    _dirac_kernels[i]->initialSetup();
}

void
DiracKernelWarehouse::timestepSetup()
{
  for (unsigned int i=0; i<_dirac_kernels.size(); i++)
    _dirac_kernels[i]->timestepSetup();
}

void
DiracKernelWarehouse::residualSetup()
{
  for (unsigned int i=0; i<_dirac_kernels.size(); i++)
    _dirac_kernels[i]->residualSetup();
}

void
DiracKernelWarehouse::jacobianSetup()
{
  for (unsigned int i=0; i<_dirac_kernels.size(); i++)
    _dirac_kernels[i]->jacobianSetup();
}

void
DiracKernelWarehouse::addDiracKernel(MooseSharedPointer<DiracKernel> & kernel)
{
  _all_ptrs.push_back(kernel);
  _dirac_kernels.push_back(kernel.get());
}
