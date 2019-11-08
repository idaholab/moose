//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorCoefDiffusion.h"

registerMooseObject("MooseTestApp", VectorCoefDiffusion);

InputParameters
VectorCoefDiffusion::validParams()
{
  InputParameters params = VectorDiffusion::validParams();
  params.addCustomTypeParam("coef", 0.0, "CoefficientType", "The coefficient of diffusion");
  return params;
}

VectorCoefDiffusion::VectorCoefDiffusion(const InputParameters & parameters)
  : VectorDiffusion(parameters), _coef(getParam<Real>("coef"))
{
}

Real
VectorCoefDiffusion::computeQpResidual()
{
  return _coef * VectorDiffusion::computeQpResidual();
}

Real
VectorCoefDiffusion::computeQpJacobian()
{
  return _coef * VectorDiffusion::computeQpJacobian();
}
