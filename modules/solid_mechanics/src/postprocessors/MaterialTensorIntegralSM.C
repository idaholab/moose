/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
