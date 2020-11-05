//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVArrayReaction.h"

registerADMooseObject("MooseApp", FVArrayReaction);

InputParameters
FVArrayReaction::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addParam<MaterialPropertyName>("coeff",
                                        "The name of the reactivity, "
                                        "can be scalar, vector, or matrix.");
  params.addClassDescription("The array reaction operator with the weak "
                             "form of $(\\psi_i, u_h)$.");
  return params;
}

FVArrayReaction::FVArrayReaction(const InputParameters & parameters)
  : FVArrayElementalKernel(parameters),
    _r(hasADMaterialProperty<Real>("coeff") ? &getADMaterialProperty<Real>("coeff") : nullptr),
    _r_array(hasADMaterialProperty<RealEigenVector>("coeff")
                 ? &getADMaterialProperty<RealEigenVector>("coeff")
                 : nullptr),
    _r_2d_array(hasADMaterialProperty<RealEigenMatrix>("coeff")
                    ? &getADMaterialProperty<RealEigenMatrix>("coeff")
                    : nullptr)
{
}

ADRealEigenVector
FVArrayReaction::computeQpResidual()
{
  if (_r)
  {
    return (*_r)[_qp] * _u[_qp];
  }
  else if (_r_array)
  {
    mooseAssert((*_r_array)[_qp].size() == _var.count(),
                "reaction_coefficient size is inconsistent with the number of components of array "
                "variable");
    return (*_r_array)[_qp].array() * _u[_qp].array();
  }
  else
  {
    mooseAssert((*_r_2d_array)[_qp].cols() == _var.count(),
                "reaction_coefficient size is inconsistent with the number of components of array "
                "variable");
    mooseAssert((*_r_2d_array)[_qp].rows() == _var.count(),
                "reaction_coefficient size is inconsistent with the number of components of array "
                "variable");
    return (*_r_2d_array)[_qp] * _u[_qp];
  }
}
