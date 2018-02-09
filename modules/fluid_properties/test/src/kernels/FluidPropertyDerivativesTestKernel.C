//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluidPropertyDerivativesTestKernel.h"
#include "MooseVariable.h"

template <>
InputParameters
validParams<FluidPropertyDerivativesTestKernel>()
{
  InputParameters params = validParams<Kernel>();

  params.addClassDescription("Base class for kernels testing derivatives of a fluid property call");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name of fluid properties user object");

  return params;
}

FluidPropertyDerivativesTestKernel::FluidPropertyDerivativesTestKernel(
    const InputParameters & parameters)
  : Kernel(parameters), _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

FluidPropertyDerivativesTestKernel::~FluidPropertyDerivativesTestKernel() {}

Real
FluidPropertyDerivativesTestKernel::computeQpJacobian()
{
  // Since this kernel can act on an arbitrary variable to achieve its purpose,
  // all variables should be treated the same - their implementations should
  // be in the same function: computeQpOffDiagJacobian().

  return computeQpOffDiagJacobian(_var.number());
}
