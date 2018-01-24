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
  params.addParam<MaterialPropertyName>("rho_name", "rho", "density name");
  params.addParam<MaterialPropertyName>("cp_name", "cp", "specific heat name");
  return params;
}

INSTemperatureTimeDerivative::INSTemperatureTimeDerivative(const InputParameters & parameters)
  : TimeDerivative(parameters),
    _rho(getMaterialProperty<Real>("rho_name")),
    _cp(getMaterialProperty<Real>("cp_name"))
{
}

Real
INSTemperatureTimeDerivative::computeQpResidual()
{
  return _rho[_qp] * _cp[_qp] * TimeDerivative::computeQpResidual();
}

Real
INSTemperatureTimeDerivative::computeQpJacobian()
{
  return _rho[_qp] * _cp[_qp] * TimeDerivative::computeQpJacobian();
}

Real
INSTemperatureTimeDerivative::computeQpOffDiagJacobian(unsigned)
{
  return 0.;
}
