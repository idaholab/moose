//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosConvectionPrecompute.h"

registerKokkosResidualObject("MooseTestApp", KokkosConvectionPrecompute);

/**
 * This function defines the valid parameters for
 * this Kernel and their default values
 */
InputParameters
KokkosConvectionPrecompute::validParams()
{
  InputParameters params = KernelValue::validParams();
  params.addRequiredParam<RealVectorValue>("velocity", "Velocity Vector");
  return params;
}

KokkosConvectionPrecompute::KokkosConvectionPrecompute(const InputParameters & parameters)
  : KernelValue(parameters), _velocity(getParam<RealVectorValue>("velocity"))
{
}
