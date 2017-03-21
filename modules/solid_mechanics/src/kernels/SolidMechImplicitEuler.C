/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SolidMechImplicitEuler.h"
#include "SubProblem.h"

template <>
InputParameters
validParams<SolidMechImplicitEuler>()
{
  InputParameters params = validParams<SecondDerivativeImplicitEuler>();
  params.addParam<Real>("artificial_scaling", "Factor to replace rho/dt^2");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

SolidMechImplicitEuler::SolidMechImplicitEuler(const InputParameters & parameters)
  : SecondDerivativeImplicitEuler(parameters),
    _density(getMaterialProperty<Real>("density")),
    _artificial_scaling_set(parameters.isParamValid("artificial_scaling")),
    _artificial_scaling(_artificial_scaling_set ? getParam<Real>("artificial_scaling") : 1)
{
}

Real
SolidMechImplicitEuler::computeQpResidual()
{
  return scaling() * _density[_qp] * SecondDerivativeImplicitEuler::computeQpResidual();
}

Real
SolidMechImplicitEuler::computeQpJacobian()
{
  return scaling() * _density[_qp] * SecondDerivativeImplicitEuler::computeQpJacobian();
}

Real
SolidMechImplicitEuler::scaling()
{
  Real factor(_artificial_scaling);
  if (_artificial_scaling_set)
  {
    factor *= _dt * _dt / _density[_qp];
  }
  return factor;
}
