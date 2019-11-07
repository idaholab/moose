//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Biharmonic.h"

registerMooseObject("MooseTestApp", Biharmonic);

InputParameters
Biharmonic::validParams()
{
  InputParameters params = Kernel::validParams();
  return params;
}

Biharmonic::Biharmonic(const InputParameters & parameters)
  : Kernel(parameters), _second_u(second()), _second_phi(secondPhi()), _second_test(secondTest())
{
}

Real
Biharmonic::computeQpResidual()
{
  return _second_u[_qp].tr() * _second_test[_i][_qp].tr();
}

Real
Biharmonic::computeQpJacobian()
{
  return _second_phi[_j][_qp].tr() * _second_test[_i][_qp].tr();
}
