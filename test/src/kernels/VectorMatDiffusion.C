//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorMatDiffusion.h"

registerMooseObject("MooseTestApp", VectorMatDiffusion);

InputParameters
VectorMatDiffusion::validParams()
{
  InputParameters params = VectorDiffusion::validParams();
  params.addParam<MaterialPropertyName>("coef", "The anisotropic (diagonal) coefficient of diffusion");
  return params;
}

VectorMatDiffusion::VectorMatDiffusion(const InputParameters & parameters)
  : VectorDiffusion(parameters), _coef(getMaterialProperty<RealVectorValue>("prop_name"))
{
}

Real
VectorMatDiffusion::computeQpResidual()
{
  return _coef[_qp](_i) * VectorDiffusion::computeQpResidual();
}

Real
VectorMatDiffusion::computeQpJacobian()
{
  if (_i == _j)
    return _coef[_qp](_i) * VectorDiffusion::computeQpJacobian();
  else
    return 0;
}
