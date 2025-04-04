//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCoefCoupledTimeDerivative.h"

registerMooseObject("PhaseFieldApp", ADCoefCoupledTimeDerivative);

InputParameters
ADCoefCoupledTimeDerivative::validParams()
{
  InputParameters params = ADCoupledTimeDerivative::validParams();
  params.addClassDescription("Scaled time derivative Kernel that acts on a coupled variable");
  params.addRequiredParam<Real>("coef", "Coefficient");
  return params;
}

ADCoefCoupledTimeDerivative::ADCoefCoupledTimeDerivative(const InputParameters & parameters)
  : ADCoupledTimeDerivative(parameters), _coef(getParam<Real>("coef"))
{
}

ADReal
ADCoefCoupledTimeDerivative::precomputeQpResidual()
{
  return ADCoupledTimeDerivative::precomputeQpResidual() * _coef;
}
