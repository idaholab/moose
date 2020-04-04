//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeVariableBaseEigenStrain.h"

registerMooseObject("TensorMechanicsApp", ComputeVariableBaseEigenStrain);

InputParameters
ComputeVariableBaseEigenStrain::validParams()
{
  InputParameters params = ComputeEigenstrainBase::validParams();
  params.addClassDescription("Computes Eigenstrain based on material property tensor base");
  params.addRequiredParam<MaterialPropertyName>("base_tensor_property_name",
                                                "Name of base tensor property");
  params.addParam<MaterialPropertyName>(
      "prefactor", 1.0, "Name of material defining the variable dependence");
  params.addParam<std::vector<Real>>(
      "offset_tensor", "Vector of values defining the constant base tensor for the Eigenstrain");
  return params;
}

ComputeVariableBaseEigenStrain::ComputeVariableBaseEigenStrain(const InputParameters & parameters)
  : ComputeEigenstrainBase(parameters),
    _base_tensor(getMaterialProperty<RealTensorValue>("base_tensor_property_name")),
    _prefactor(getMaterialProperty<Real>("prefactor"))
{
  if (isParamValid("offset_tensor"))
    _offset_tensor.fillFromInputVector(getParam<std::vector<Real>>("offset_tensor"));
  else
    _offset_tensor.zero();
}

void
ComputeVariableBaseEigenStrain::computeQpEigenstrain()
{
  RankTwoTensor base_rank_two_tensor = _base_tensor[_qp];
  _eigenstrain[_qp] = base_rank_two_tensor * _prefactor[_qp] + _offset_tensor;
}
