//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorCoupledTimeDerivative.h"

registerMooseObject("MooseApp", VectorCoupledTimeDerivative);

InputParameters
VectorCoupledTimeDerivative::validParams()
{
  InputParameters params = VectorKernel::validParams();
  params.addClassDescription(
      "Time derivative Kernel that acts on a coupled vector variable. Weak form: "
      "$(\\vec{\\psi}_i, \\frac{\\partial \\vec{v_h}}{\\partial t})$.");
  params.addRequiredCoupledVar("v", "Coupled vector variable");
  return params;
}

VectorCoupledTimeDerivative::VectorCoupledTimeDerivative(const InputParameters & parameters)
  : VectorKernel(parameters),
    _v_dot(coupledVectorDot("v")),
    _dv_dot(coupledVectorDotDu("v")),
    _v_var(coupled("v"))
{
}

Real
VectorCoupledTimeDerivative::computeQpResidual()
{
  return _test[_i][_qp] * _v_dot[_qp];
}

Real
VectorCoupledTimeDerivative::computeQpJacobian()
{
  return 0.0;
}

Real
VectorCoupledTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _v_var)
    return _test[_i][_qp] * _phi[_j][_qp] * _dv_dot[_qp];

  return 0.0;
}
