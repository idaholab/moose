//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosNodalL2Norm.h"

registerKokkosPostprocessor("MooseApp", KokkosNodalL2Norm);

InputParameters
KokkosNodalL2Norm::validParams()
{
  InputParameters params = KokkosNodalSum::validParams();
  params.addClassDescription(
      "Computes the nodal L2-norm of the coupled variable, which is defined by summing the square "
      "of its value at every node and taking the square root.");
  return params;
}

KokkosNodalL2Norm::KokkosNodalL2Norm(const InputParameters & parameters)
  : KokkosNodalSum(parameters)
{
}

Real
KokkosNodalL2Norm::getValue() const
{
  return std::sqrt(_reduction_buffer(0));
}
