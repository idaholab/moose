//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVQpFluxKernel.h"

InputParameters
FVQpFluxKernel::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  return params;
}

FVQpFluxKernel::FVQpFluxKernel(const InputParameters & params)
  : FVFluxKernel(params), _u_elem(_var.adSln()), _u_neighbor(_var.adSlnNeighbor())
{
}
