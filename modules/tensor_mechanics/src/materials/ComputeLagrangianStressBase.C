#include "ComputeLagrangianStressBase.h"

InputParameters
ComputeLagrangianStressBase::validParams()
{
  InputParameters params = Material::validParams();

  params.addParam<bool>(
      "large_kinematics", false, "Use large displacement kinematics in the kernel.");
  return params;
}

ComputeLagrangianStressBase::ComputeLagrangianStressBase(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _ld(getParam<bool>("large_kinematics")),
    _cauchy_stress(declareProperty<RankTwoTensor>("stress")),
    _cauchy_jacobian(declareProperty<RankFourTensor>("cauchy_jacobian")),
    _pk1_stress(declareProperty<RankTwoTensor>("pk1_stress")),
    _pk1_jacobian(declareProperty<RankFourTensor>("pk1_jacobian"))
{
}

void
ComputeLagrangianStressBase::initQpStatefulProperties()
{
  _cauchy_stress[_qp].zero();
  _cauchy_jacobian[_qp].zero();
  _pk1_stress[_qp].zero();
  _pk1_jacobian[_qp].zero();
}

void
ComputeLagrangianStressBase::computeQpProperties()
{
  computeQpStressUpdate();
}
