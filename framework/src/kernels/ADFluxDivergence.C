//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADFluxDivergence.h"

registerMooseObject("MooseApp", ADFluxDivergence);

InputParameters
ADFluxDivergence::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Computes divergence of a flux vector provided by a material.");
  params.addParam<MaterialPropertyName>(
      "flux", "flux", "Name of the flux vector material property");
  return params;
}

ADFluxDivergence::ADFluxDivergence(const InputParameters & parameters)
  : ADKernel(parameters), _flux(getADMaterialProperty<RealVectorValue>("flux"))
{
}

ADReal
ADFluxDivergence::computeQpResidual()
{
  return -_grad_test[_i][_qp] * _flux[_qp];
}
