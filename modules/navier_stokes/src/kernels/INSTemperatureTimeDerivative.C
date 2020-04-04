//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSTemperatureTimeDerivative.h"

registerMooseObject("NavierStokesApp", INSTemperatureTimeDerivative);

InputParameters
INSTemperatureTimeDerivative::validParams()
{
  InputParameters params = TimeDerivative::validParams();
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
