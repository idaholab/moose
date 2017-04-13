/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "INSTemperatureTimeDerivative.h"

template <>
InputParameters
validParams<INSTemperatureTimeDerivative>()
{
  InputParameters params = validParams<TimeDerivative>();
  params.addClassDescription("This class computes the time derivative for the incompressible "
                             "Navier-Stokes momentum equation.");
  params.addRequiredParam<Real>("rho", "density");
  params.addRequiredParam<Real>("cp", "specific heat");
  return params;
}

INSTemperatureTimeDerivative::INSTemperatureTimeDerivative(const InputParameters & parameters)
  : TimeDerivative(parameters), _rho(getParam<Real>("rho")), _cp(getParam<Real>("cp"))
{
}

Real
INSTemperatureTimeDerivative::computeQpResidual()
{
  return _rho * _cp * TimeDerivative::computeQpResidual();
}

Real
INSTemperatureTimeDerivative::computeQpJacobian()
{
  return _rho * _cp * TimeDerivative::computeQpJacobian();
}

Real
INSTemperatureTimeDerivative::computeQpOffDiagJacobian(unsigned)
{
  return 0.;
}
