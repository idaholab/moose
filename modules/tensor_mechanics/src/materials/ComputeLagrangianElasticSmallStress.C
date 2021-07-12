#include "ComputeLagrangianElasticSmallStress.h"

registerMooseObject("TensorMechanicsApp", ComputeLagrangianElasticSmallStress);

InputParameters
ComputeLagrangianElasticSmallStress::validParams()
{
  InputParameters params = ComputeLagrangianStressSmall::validParams();

  params.addParam<MaterialPropertyName>(
      "elasticity_tensor", "elasticity_tensor", "The name of the elasticity tensor.");

  return params;
}

ComputeLagrangianElasticSmallStress::ComputeLagrangianElasticSmallStress(
    const InputParameters & parameters)
  : ComputeLagrangianStressSmall(parameters),
    _elasticity_tensor(
        getMaterialProperty<RankFourTensor>(getParam<MaterialPropertyName>("elasticity_tensor")))
{
}

void
ComputeLagrangianElasticSmallStress::computeQpSmallStress()
{
  _small_stress[_qp] = _elasticity_tensor[_qp] * _mechanical_strain[_qp];
  _small_jacobian[_qp] = _elasticity_tensor[_qp];
}
