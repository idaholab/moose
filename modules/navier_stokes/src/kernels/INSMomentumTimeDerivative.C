/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "INSMomentumTimeDerivative.h"

template <>
InputParameters
validParams<INSMomentumTimeDerivative>()
{
  InputParameters params = validParams<TimeDerivative>();
  params.addClassDescription("This class computes the time derivative for the incompressible "
                             "Navier-Stokes momentum equation.");
  params.addRequiredParam<Real>("rho", "density");
  return params;
}

INSMomentumTimeDerivative::INSMomentumTimeDerivative(const InputParameters & parameters)
  : TimeDerivative(parameters), _rho(getParam<Real>("rho"))
{
}

Real
INSMomentumTimeDerivative::computeQpResidual()
{
  return _rho * TimeDerivative::computeQpResidual();
}

Real
INSMomentumTimeDerivative::computeQpJacobian()
{
  return _rho * TimeDerivative::computeQpJacobian();
}

Real
INSMomentumTimeDerivative::computeQpOffDiagJacobian(unsigned)
{
  return 0.;
}
