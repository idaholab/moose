//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADThermoDiffusion.h"

registerMooseObject("MiscApp", ADThermoDiffusion);

InputParameters
ADThermoDiffusion::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription(
      "Calculates diffusion due to temperature gradient and Soret Coefficient");
  params.addRequiredCoupledVar("temperature", "The coupled temperature variable.");
  params.addParam<MaterialPropertyName>("soret_coefficient",
                                        "soret_coefficient",
                                        "The name of the Soret coefficient material property");
  return params;
}

ADThermoDiffusion::ADThermoDiffusion(const InputParameters & parameters)
  : ADKernel(parameters),
    _grad_temp(adCoupledGradient("temperature")),
    _soret_coeff(getADMaterialProperty<Real>("soret_coefficient"))
{
}

ADReal
ADThermoDiffusion::computeQpResidual()
{
  return _soret_coeff[_qp] * _grad_temp[_qp] * _grad_test[_i][_qp];
}
