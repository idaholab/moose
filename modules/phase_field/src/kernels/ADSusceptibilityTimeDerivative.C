//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADSusceptibilityTimeDerivative.h"

registerMooseObject("PhaseFieldApp", ADSusceptibilityTimeDerivative);

InputParameters
ADSusceptibilityTimeDerivative::validParams()
{
  InputParameters params = ADTimeDerivative::validParams();
  params.addClassDescription(
      "A modified time derivative Kernel that multiplies the time derivative "
      "of a variable by a generalized susceptibility");
  params.addRequiredParam<MaterialPropertyName>(
      "f_name", "Susceptibility function F defined in a FunctionMaterial");
  return params;
}

ADSusceptibilityTimeDerivative::ADSusceptibilityTimeDerivative(const InputParameters & parameters)
  : ADTimeDerivative(parameters), _Chi(getADMaterialProperty<Real>("f_name"))
{
}

ADReal
ADSusceptibilityTimeDerivative::precomputeQpResidual()
{
  return _u_dot[_qp] * _Chi[_qp];
}
