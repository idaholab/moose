//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideFluxIntegral.h"

template <>
InputParameters
validParams<SideFluxIntegral>()
{
  InputParameters params = validParams<SideIntegralVariablePostprocessor>();
  params.addRequiredParam<MaterialPropertyName>(
      "diffusivity",
      "The name of the diffusivity material property that will be used in the flux computation.");
  params.addClassDescription("Computes the integral of the flux over the specified boundary");
  return params;
}

SideFluxIntegral::SideFluxIntegral(const InputParameters & parameters)
  : SideIntegralVariablePostprocessor(parameters),
    _diffusivity(parameters.get<MaterialPropertyName>("diffusivity")),
    _diffusion_coef(getMaterialProperty<Real>(_diffusivity))
{
}

Real
SideFluxIntegral::computeQpIntegral()
{
  return -_diffusion_coef[_qp] * _grad_u[_qp] * _normals[_qp];
}
