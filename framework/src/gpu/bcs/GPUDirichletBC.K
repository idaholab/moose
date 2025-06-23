//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUDirichletBC.h"

registerMooseObjectAliased("MooseApp", GPUDirichletBCKernel, "GPUDirichletBC");

InputParameters
GPUDirichletBCKernel::validParams()
{
  InputParameters params = GPUDirichletBC::validParams();
  params.addClassDescription("Imposes the essential boundary condition $u=g$, where $g$ "
                             "is a constant, controllable value.");
  return params;
}

GPUDirichletBCKernel::GPUDirichletBCKernel(const InputParameters & parameters)
  : GPUDirichletBC(parameters)
{
}
