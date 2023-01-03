//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayReaction.h"

registerMooseObject("MooseApp", ArrayReaction);

InputParameters
ArrayReaction::validParams()
{
  InputParameters params = ArrayKernel::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "reaction_coefficient",
      "The name of the reactivity, can be scalar, vector, or matrix material property.");
  params.addClassDescription("The array reaction operator with the weak "
                             "form of $(\\psi_i, u_h)$.");
  return params;
}

ArrayReaction::ArrayReaction(const InputParameters & parameters)
  : ArrayKernel(parameters),
    _r(hasMaterialProperty<Real>("reaction_coefficient")
           ? &getMaterialProperty<Real>("reaction_coefficient")
           : nullptr),
    _r_array(hasMaterialProperty<RealEigenVector>("reaction_coefficient")
                 ? &getMaterialProperty<RealEigenVector>("reaction_coefficient")
                 : nullptr),
    _r_2d_array(hasMaterialProperty<RealEigenMatrix>("reaction_coefficient")
                    ? &getMaterialProperty<RealEigenMatrix>("reaction_coefficient")
                    : nullptr)
{
  if (!_r && !_r_array && !_r_2d_array)
  {
    MaterialPropertyName mat = getParam<MaterialPropertyName>("reaction_coefficient");
    mooseError("Property " + mat + " is of unsupported type for ArrayReaction");
  }
}

void
ArrayReaction::computeQpResidual(RealEigenVector & residual)
{

  if (_r)
    residual = (*_r)[_qp] * _u[_qp] * _test[_i][_qp];

  else if (_r_array)
  {
    mooseAssert((*_r_array)[_qp].size() == _var.count(),
                "reaction_coefficient size is inconsistent with the number of components of array "
                "variable");
    // WARNING: use noalias() syntax with caution. See ArrayDiffusion.C for more details.
    residual.noalias() = (*_r_array)[_qp].asDiagonal() * _u[_qp] * _test[_i][_qp];
  }

  else
  {
    mooseAssert((*_r_2d_array)[_qp].cols() == _var.count(),
                "reaction_coefficient size is inconsistent with the number of components of array "
                "variable");
    mooseAssert((*_r_2d_array)[_qp].rows() == _var.count(),
                "reaction_coefficient size is inconsistent with the number of components of array "
                "variable");
    // WARNING: use noalias() syntax with caution. See ArrayDiffusion.C for more details.
    residual.noalias() = (*_r_2d_array)[_qp] * _u[_qp] * _test[_i][_qp];
  }
}

RealEigenVector
ArrayReaction::computeQpJacobian()
{
  if (_r)
    return RealEigenVector::Constant(_var.count(), _phi[_j][_qp] * _test[_i][_qp] * (*_r)[_qp]);
  else if (_r_array)
    return _phi[_j][_qp] * _test[_i][_qp] * (*_r_array)[_qp];

  else
    return _phi[_j][_qp] * _test[_i][_qp] * (*_r_2d_array)[_qp].diagonal();
}

RealEigenMatrix
ArrayReaction::computeQpOffDiagJacobian(const MooseVariableFEBase & jvar)
{
  if (jvar.number() == _var.number() && _r_2d_array)
    return _phi[_j][_qp] * _test[_i][_qp] * (*_r_2d_array)[_qp];
  else
    return ArrayKernel::computeQpOffDiagJacobian(jvar);
}
