//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorMatDiffusion.h"

registerMooseObject("MooseTestApp", FunctorMatDiffusion);

InputParameters
FunctorMatDiffusion::validParams()
{
  auto params = ADKernel::validParams();
  params.addParam<MaterialPropertyName>(
      "diffusivity", "D", "The diffusivity value or material property");
  return params;
}

FunctorMatDiffusion::FunctorMatDiffusion(const InputParameters & parameters)
  : ADKernel(parameters), _diff(getFunctorMaterialProperty<Real>("diffusivity"))
{
}

ADReal
FunctorMatDiffusion::computeQpResidual()
{
  return _diff(std::make_tuple(_current_elem, _qp, _qrule)) * _grad_test[_i][_qp] * _grad_u[_qp];
}
