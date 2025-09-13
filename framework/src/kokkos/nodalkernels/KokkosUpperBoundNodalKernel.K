//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosUpperBoundNodalKernel.h"

registerKokkosResidualObject("MooseApp", KokkosUpperBoundNodalKernel);

InputParameters
KokkosUpperBoundNodalKernel::validParams()
{
  InputParameters params = KokkosBoundNodalKernel::validParams();
  params.addClassDescription("Used to prevent a coupled variable from going above a upper bound");
  params.addParam<Real>("upper_bound", 0, "The upper bound on the coupled variable");
  return params;
}

KokkosUpperBoundNodalKernel::KokkosUpperBoundNodalKernel(const InputParameters & parameters)
  : KokkosBoundNodalKernel(parameters), _upper_bound(getParam<Real>("upper_bound"))
{
}
