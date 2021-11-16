#include "ComputeLagrangianElasticEngineeringStress.h"

registerMooseObject("TensorMechanicsApp", ComputeLagrangianElasticEngineeringStress);

InputParameters
ComputeLagrangianElasticEngineeringStress::validParams()
{
  InputParameters params = ComputeLagrangianObjectiveStress::validParams();

  params.addParam<MaterialPropertyName>(
      "elasticity_tensor", "elasticity_tensor", "The name of the elasticity tensor.");

  return params;
}

ComputeLagrangianElasticEngineeringStress::ComputeLagrangianElasticEngineeringStress(
    const InputParameters & parameters)
  : ComputeLagrangianObjectiveStress(parameters),
    _elasticity_tensor(
        getMaterialProperty<RankFourTensor>(getParam<MaterialPropertyName>("elasticity_tensor")))
{
}

void
ComputeLagrangianElasticEngineeringStress::computeQpSmallStress()
{
  _small_stress[_qp] = _elasticity_tensor[_qp] * _mechanical_strain[_qp];
  _small_jacobian[_qp] = _elasticity_tensor[_qp];
}
