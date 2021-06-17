//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MallocKernel.h"
#include "SystemBase.h"

registerMooseObject("NavierStokesTestApp", MallocKernel);

InputParameters
MallocKernel::validParams()
{
  return Kernel::validParams();
}

MallocKernel::MallocKernel(const InputParameters & parameters) : Kernel(parameters) {}

void
MallocKernel::jacobianSetup()
{
  auto & sys_mat = _sys.getMatrix(_sys.systemMatrixTag());

  // Add to an off-diagional
  sys_mat.add(0, sys_mat.n() - 1, 0);
}

Real
MallocKernel::computeQpResidual()
{
  return 0;
}
