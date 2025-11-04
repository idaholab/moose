//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosLowerBoundNodalKernel.h"

registerKokkosResidualObject("MooseApp", KokkosLowerBoundNodalKernel);

InputParameters
KokkosLowerBoundNodalKernel::validParams()
{
  InputParameters params = KokkosBoundNodalKernel::validParams();
  params.addClassDescription("Used to prevent a coupled variable from going below a lower bound");
  params.addParam<Real>("lower_bound", 0, "The lower bound on the coupled variable");
  return params;
}

KokkosLowerBoundNodalKernel::KokkosLowerBoundNodalKernel(const InputParameters & parameters)
  : KokkosBoundNodalKernel(parameters), _lower_bound(getParam<Real>("lower_bound"))
{
}
