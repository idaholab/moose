//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayTimeDerivative.h"

registerMooseObject("MooseApp", ArrayTimeDerivative);

InputParameters
ArrayTimeDerivative::validParams()
{
  InputParameters params = ArrayTimeKernel::validParams();
  params.addClassDescription("Array time derivative operator with the weak form of $(\\psi_i, "
                             "\\frac{\\partial u_h}{\\partial t})$.");
  params.addParam<MaterialPropertyName>("time_derivative_coefficient",
                                        "The name of the time derivative coefficient. "
                                        "Can be scalar, vector, or matrix material property.");
  return params;
}

ArrayTimeDerivative::ArrayTimeDerivative(const InputParameters & parameters)
  : ArrayTimeKernel(parameters),
    _has_coefficient(isParamValid("time_derivative_coefficient")),
    _coeff(_has_coefficient && hasMaterialProperty<Real>("time_derivative_coefficient")
               ? &getMaterialProperty<Real>("time_derivative_coefficient")
               : nullptr),
    _coeff_array(_has_coefficient &&
                         hasMaterialProperty<RealEigenVector>("time_derivative_coefficient")
                     ? &getMaterialProperty<RealEigenVector>("time_derivative_coefficient")
                     : nullptr),
    _coeff_2d_array(_has_coefficient &&
                            hasMaterialProperty<RealEigenMatrix>("time_derivative_coefficient")
                        ? &getMaterialProperty<RealEigenMatrix>("time_derivative_coefficient")
                        : nullptr)
{
  if (!_coeff && !_coeff_array && !_coeff_2d_array && _has_coefficient)
  {
    MaterialPropertyName mat = getParam<MaterialPropertyName>("time_derivative_coefficient");
    mooseError("Property " + mat + " is of unsupported type for ArrayTimeDerivative");
  }
}

void
ArrayTimeDerivative::computeQpResidual(RealEigenVector & residual)
{
  if (!_has_coefficient)
    residual = _u_dot[_qp] * _test[_i][_qp];
  else if (_coeff)
    residual = (*_coeff)[_qp] * _u_dot[_qp] * _test[_i][_qp];
  else if (_coeff_array)
  {
    mooseAssert((*_coeff_array)[_qp].size() == _var.count(),
                "time_derivative_coefficient size is inconsistent with the number of components "
                "in array variable");
    // WARNING: use noalias() syntax with caution. See ArrayDiffusion.C for more details.
    residual.noalias() = (*_coeff_array)[_qp].asDiagonal() * _u_dot[_qp] * _test[_i][_qp];
  }
  else
  {
    mooseAssert((*_coeff_2d_array)[_qp].cols() == _var.count(),
                "time_derivative_coefficient size is inconsistent with the number of components "
                "in array variable");
    mooseAssert((*_coeff_2d_array)[_qp].rows() == _var.count(),
                "time_derivative_coefficient size is inconsistent with the number of components "
                "in array variable");
    // WARNING: use noalias() syntax with caution. See ArrayDiffusion.C for more details.
    residual.noalias() = (*_coeff_2d_array)[_qp] * _u_dot[_qp] * _test[_i][_qp];
  }
}

RealEigenVector
ArrayTimeDerivative::computeQpJacobian()
{
  Real tmp = _test[_i][_qp] * _phi[_j][_qp] * _du_dot_du[_qp];
  if (!_has_coefficient)
    return RealEigenVector::Constant(_var.count(), tmp);
  else if (_coeff)
    return RealEigenVector::Constant(_var.count(), tmp * (*_coeff)[_qp]);
  else if (_coeff_array)
    return tmp * (*_coeff_array)[_qp];
  else
    return tmp * (*_coeff_2d_array)[_qp].diagonal();
}

RealEigenMatrix
ArrayTimeDerivative::computeQpOffDiagJacobian(const MooseVariableFEBase & jvar)
{
  if (jvar.number() == _var.number() && _coeff_2d_array)
    return _phi[_j][_qp] * _test[_i][_qp] * _du_dot_du[_qp] * (*_coeff_2d_array)[_qp];
  else
    return ArrayKernel::computeQpOffDiagJacobian(jvar);
}
