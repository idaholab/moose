//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SecondTimeDerivative.h"

// MOOSE includes
#include "MooseVariableFE.h"

registerMooseObject("MooseApp", SecondTimeDerivative);

template <>
InputParameters
validParams<SecondTimeDerivative>()
{
  InputParameters params = validParams<TimeDerivative>();
  params.addClassDescription("The second time derivative operator with the weak form of $(\\psi_i, "
                             "\\frac{\\partial^2 u_h}{\\partial t^2})$.");
  params.set<std::vector<TagName>>("extra_matrix_tags") = {"SecondTime"};
  return params;
}

SecondTimeDerivative::SecondTimeDerivative(const InputParameters & parameters)
  : TimeDerivative(parameters)
{
  _u_dotdot = &(_var.uDotDot());
  _du_dotdot_du = &(_var.duDotDotDu());
}

Real
SecondTimeDerivative::computeQpResidual()
{
  return _test[_i][_qp] * (*_u_dotdot)[_qp];
}

Real
SecondTimeDerivative::computeQpJacobian()
{
  return _test[_i][_qp] * _phi[_j][_qp] * (*_du_dotdot_du)[_qp];
}
