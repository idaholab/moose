//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoefCoupledTimeDerivative.h"

registerMooseObject("PhaseFieldApp", CoefCoupledTimeDerivative);

InputParameters
CoefCoupledTimeDerivative::validParams()
{
  InputParameters params = CoupledTimeDerivative::validParams();
  params.addClassDescription("Scaled time derivative Kernel that acts on a coupled variable");
  params.addRequiredParam<Real>("coef", "Coefficient");
  return params;
}

CoefCoupledTimeDerivative::CoefCoupledTimeDerivative(const InputParameters & parameters)
  : CoupledTimeDerivative(parameters), _coef(getParam<Real>("coef"))
{
}

Real
CoefCoupledTimeDerivative::computeQpResidual()
{
  return CoupledTimeDerivative::computeQpResidual() * _coef;
}

Real
CoefCoupledTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  return CoupledTimeDerivative::computeQpOffDiagJacobian(jvar) * _coef;
}
