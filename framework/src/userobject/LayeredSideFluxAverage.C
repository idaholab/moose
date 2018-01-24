//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LayeredSideFluxAverage.h"

template <>
InputParameters
validParams<LayeredSideFluxAverage>()
{
  InputParameters params = validParams<LayeredSideIntegral>();
  params.addRequiredParam<std::string>(
      "diffusivity",
      "The name of the diffusivity material property that will be used in the flux computation.");
  return params;
}

LayeredSideFluxAverage::LayeredSideFluxAverage(const InputParameters & parameters)
  : LayeredSideAverage(parameters),
    _diffusivity(parameters.get<std::string>("diffusivity")),
    _diffusion_coef(getMaterialProperty<Real>(_diffusivity))
{
}

Real
LayeredSideFluxAverage::computeQpIntegral()
{
  return -_diffusion_coef[_qp] * _grad_u[_qp] * _normals[_qp];
}
