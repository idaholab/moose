//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADArrayReaction.h"

registerMooseObject("MooseApp", ADArrayReaction);

InputParameters
ADArrayReaction::validParams()
{
  InputParameters params = ADArrayKernel::validParams();
  params += ADFunctorInterface::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "reaction_coefficient",
      "The name of the reaction, can be scalar, vector, or matrix material property.");
  params.addClassDescription(
      "An automatic differentiation kernel for the array reaction operator ($u$)");
  return params;
}

ADArrayReaction::ADArrayReaction(const InputParameters & parameters)
  : ADArrayKernel(parameters),
    _r(hasMaterialProperty<ADReal>("reaction_coefficient")
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
    mooseError("Property '" + mat + "' is of unsupported type for ADArrayReaction");
  }
}

void
ADArrayReaction::computeQpResidual(ADRealEigenVector & residual)
{
  if (_r)
    residual = (*_r)[_qp] * _u[_qp] * _test[_i][_qp];
  else if (_r_array)
    residual.noalias() = (*_r_array)[_qp].cwiseProduct(_u[_qp]) * _test[_i][_qp];
  else
    residual.noalias() = (*_r_2d_array)[_qp] * _u[_qp] * _test[_i][_qp];
}
