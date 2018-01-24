//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialTensorIntegralSM.h"

#include "SymmTensor.h"
#include "MaterialTensorCalculator.h"

template <>
InputParameters
validParams<MaterialTensorIntegralSM>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();
  params += validParams<MaterialTensorCalculator>();
  params.addRequiredParam<std::string>("tensor", "The material tensor name.");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

MaterialTensorIntegralSM::MaterialTensorIntegralSM(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _material_tensor_calculator(parameters),
    _tensor(getMaterialProperty<SymmTensor>(getParam<std::string>("tensor")))
{
}

Real
MaterialTensorIntegralSM::computeQpIntegral()
{
  RealVectorValue direction;
  return _material_tensor_calculator.getTensorQuantity(_tensor[_qp], _q_point[_qp], direction);
}
