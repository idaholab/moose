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
  params.addParam<MaterialPropertyName>("rho_name", "rho", "density name");
  return params;
}

INSMomentumTimeDerivative::INSMomentumTimeDerivative(const InputParameters & parameters)
  : TimeDerivative(parameters), _rho(getMaterialProperty<Real>("rho_name"))
{
}

Real
INSMomentumTimeDerivative::computeQpResidual()
{
  return _rho[_qp] * TimeDerivative::computeQpResidual();
}

Real
INSMomentumTimeDerivative::computeQpJacobian()
{
  return _rho[_qp] * TimeDerivative::computeQpJacobian();
}

Real
INSMomentumTimeDerivative::computeQpOffDiagJacobian(unsigned)
{
  return 0.;
}
