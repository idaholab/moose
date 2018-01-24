/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeVariableBaseEigenStrain.h"

template <>
InputParameters
validParams<ComputeVariableBaseEigenStrain>()
{
  InputParameters params = validParams<ComputeEigenstrainBase>();
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
