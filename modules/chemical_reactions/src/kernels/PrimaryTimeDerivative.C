/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PrimaryTimeDerivative.h"

template <>
InputParameters
validParams<PrimaryTimeDerivative>()
{
  InputParameters params = validParams<TimeDerivative>();
  params.addClassDescription("Derivative of primary species concentration wrt time");
  return params;
}

PrimaryTimeDerivative::PrimaryTimeDerivative(const InputParameters & parameters)
  : TimeDerivative(parameters), _porosity(getMaterialProperty<Real>("porosity"))
{
}

Real
PrimaryTimeDerivative::computeQpResidual()
{
  return _porosity[_qp] * TimeDerivative::computeQpResidual();
}

Real
PrimaryTimeDerivative::computeQpJacobian()
{
  return _porosity[_qp] * TimeDerivative::computeQpJacobian();
}
