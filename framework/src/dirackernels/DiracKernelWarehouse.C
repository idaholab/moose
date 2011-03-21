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

DiracKernelWarehouse::DiracKernelWarehouse()
{
}

DiracKernelWarehouse::~DiracKernelWarehouse()
{
  // delete  DiracKernels
  for (std::vector<DiracKernel *>::iterator i=_dirac_kernels.begin(); i!=_dirac_kernels.end(); ++i)
    delete *i;
}

DiracKernelIterator
DiracKernelWarehouse::diracKernelsBegin()
{
  return _dirac_kernels.begin();
}

DiracKernelIterator
DiracKernelWarehouse::diracKernelsEnd()
{
  return _dirac_kernels.end();
}

void
DiracKernelWarehouse::addDiracKernel(DiracKernel *DiracKernel)
{
  _dirac_kernels.push_back(DiracKernel);
}
