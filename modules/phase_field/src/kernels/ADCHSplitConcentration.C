//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCHSplitConcentration.h"

registerMooseObject("PhaseFieldApp", ADCHSplitConcentration);

InputParameters
ADCHSplitConcentration::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Concentration kernel in Split Cahn-Hilliard that solves chemical "
                             "potential in a weak form");
  params.addRequiredCoupledVar("chemical_potential_var", "Chemical potential variable");
  params.addRequiredParam<MaterialPropertyName>("mobility", "Mobility property name");
  return params;
}

ADCHSplitConcentration::ADCHSplitConcentration(const InputParameters & parameters)
  : ADKernel(parameters),
    _grad_mu(adCoupledGradient("chemical_potential_var")),
    _mobility(getADMaterialProperty<Real>("mobility"))
{
}

ADReal
ADCHSplitConcentration::computeQpResidual()
{
  return _mobility[_qp] * _grad_mu[_qp] * _grad_test[_i][_qp];
}
