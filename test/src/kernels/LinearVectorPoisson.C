//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearVectorPoisson.h"
#include "Function.h"

registerMooseObject("MooseTestApp", LinearVectorPoisson);

InputParameters
LinearVectorPoisson::validParams()
{
  InputParameters params = VectorKernel::validParams();
  params.addParam<FunctionName>("x_exact_sln", 0, "The x forcing function.");
  params.addParam<FunctionName>("y_exact_sln", 0, "The y forcing function.");
  return params;
}

LinearVectorPoisson::LinearVectorPoisson(const InputParameters & parameters)
  : VectorKernel(parameters),
    _x_sln(getFunction("x_exact_sln")),
    _y_sln(getFunction("y_exact_sln")),
    _eps(1e-3)
{
}

Real
LinearVectorPoisson::computeQpResidual()
{
  const Real x = _q_point[_qp](0);
  const Real y = _q_point[_qp](1);

  const Real fx = -(_x_sln.value(_t, x, y - _eps) + _x_sln.value(_t, x, y + _eps) +
                    _x_sln.value(_t, x - _eps, y) + _x_sln.value(_t, x + _eps, y) -
                    4. * _x_sln.value(_t, x, y)) /
                  _eps / _eps;

  const Real fy = -(_y_sln.value(_t, x, y - _eps) + _y_sln.value(_t, x, y + _eps) +
                    _y_sln.value(_t, x - _eps, y) + _y_sln.value(_t, x + _eps, y) -
                    4. * _y_sln.value(_t, x, y)) /
                  _eps / _eps;

  return -RealVectorValue(fx, fy, 0) * _test[_i][_qp];
}

Real
LinearVectorPoisson::computeQpJacobian()
{
  return _grad_test[_i][_qp].contract(_grad_phi[_j][_qp]);
}
