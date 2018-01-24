/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CoefCoupledTimeDerivative.h"

template <>
InputParameters
validParams<CoefCoupledTimeDerivative>()
{
  InputParameters params = validParams<CoupledTimeDerivative>();
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
