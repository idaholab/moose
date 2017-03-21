/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CoefTimeDerivative.h"

template <>
InputParameters
validParams<CoefTimeDerivative>()
{
  InputParameters params = validParams<TimeDerivative>();
  params.addParam<Real>("Coefficient", 1, "The coefficient for the time derivative kernel");
  return params;
}

CoefTimeDerivative::CoefTimeDerivative(const InputParameters & parameters)
  : TimeDerivative(parameters), _coef(getParam<Real>("Coefficient"))
{
}

Real
CoefTimeDerivative::computeQpResidual()
{
  // We're reusing the TimeDerivative Kernel's residual
  // so that we don't have to recode that.
  return _coef * TimeDerivative::computeQpResidual();
}

Real
CoefTimeDerivative::computeQpJacobian()
{
  return _coef * TimeDerivative::computeQpJacobian();
}
