//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayCoupledTimeDerivative.h"

registerMooseObject("MooseApp", ArrayCoupledTimeDerivative);

InputParameters
ArrayCoupledTimeDerivative::validParams()
{
  InputParameters params = ArrayKernel::validParams();
  params.addClassDescription(
      "Time derivative Array Kernel that acts on a coupled variable. Weak form: "
      "$(\\psi_i, \\frac{\\partial v_h}{\\partial t})$. The coupled variable and "
      "the variable must have the same dimensionality");
  params.addRequiredCoupledVar("v", "Coupled variable");
  return params;
}

ArrayCoupledTimeDerivative::ArrayCoupledTimeDerivative(const InputParameters & parameters)
  : ArrayKernel(parameters),
    _v_dot(coupledArrayDot("v")),
    _dv_dot(coupledArrayDotDu("v")),
    _v_var(coupled("v"))
{
}

void
ArrayCoupledTimeDerivative::computeQpResidual(RealEigenVector & residual)
{
  mooseAssert(_var.count() == _v_dot.size(),
              "The variable and coupled variable have unequal sizes:\n  variable size        : " +
                  std::to_string(_var.count()) + "\n" +
                  "  coupled variable size: " + std::to_string(_v_dot.size()));

  residual = _test[_i][_qp] * _v_dot[_qp];
}

RealEigenVector
ArrayCoupledTimeDerivative::computeQpJacobian()
{
  return RealEigenVector::Zero(_var.count());
}

RealEigenMatrix
ArrayCoupledTimeDerivative::computeQpOffDiagJacobian(const MooseVariableFEBase & jvar)
{
  if (jvar.number() == _v_var)
    return _test[_i][_qp] * _phi[_j][_qp] * _dv_dot[_qp] *
           RealEigenVector::Ones(jvar.count()).asDiagonal();

  return RealEigenMatrix::Zero(_var.count(), jvar.count());
}
