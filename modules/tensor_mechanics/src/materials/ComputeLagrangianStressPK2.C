#include "ComputeLagrangianStressPK2.h"

InputParameters
ComputeLagrangianStressPK2::validParams()
{
  InputParameters params = ComputeLagrangianStressPK1::validParams();
  return params;
}

ComputeLagrangianStressPK2::ComputeLagrangianStressPK2(const InputParameters & parameters)
  : ComputeLagrangianStressPK1(parameters),
    _E(declareProperty<RankTwoTensor>(_base_name + "green_lagrange_strain")),
    _S(declareProperty<RankTwoTensor>(_base_name + "pk2_stress")),
    _C(declareProperty<RankFourTensor>(_base_name + "pk2_jaobian"))
{
}

void
ComputeLagrangianStressPK2::initQpStatefulProperties()
{
  ComputeLagrangianStressBase::initQpStatefulProperties();
  _E[_qp].zero();
  _S[_qp].zero();
  _C[_qp].zero();
}

void
ComputeLagrangianStressPK2::computeQpPK1Stress()
{
  // Calculate the green-lagrange strain for the benefit of the subclasses
  _E[_qp] = 0.5 * (_F[_qp].transpose() * _F[_qp] - RankTwoTensor::Identity());

  // PK2 update
  computeQpPK2Stress();

  // Complicated wrapping, see documentation
  if (_large_kinematics)
  {
    _pk1_stress[_qp] = _F[_qp] * _S[_qp];

    RankFourTensor dE = 0.5 * (RankTwoTensor::Identity().mixedProductIlJk(_F[_qp].transpose()) +
                               _F[_qp].transpose().mixedProductIkJl(RankTwoTensor::Identity()));
    auto CdE = _C[_qp] * dE;

    _pk1_jacobian[_qp] = RankTwoTensor::Identity().mixedProductIkJl(_S[_qp].transpose());

    for (size_t i = 0; i < 3; i++)
      for (size_t j = 0; j < 3; j++)
        for (size_t k = 0; k < 3; k++)
          for (size_t l = 0; l < 3; l++)
            for (size_t m = 0; m < 3; m++)
              _pk1_jacobian[_qp](i, j, k, l) += _F[_qp](i, m) * CdE(m, j, k, l);
  }
  // Small deformations all are equivalent
  else
  {
    _pk1_stress[_qp] = _S[_qp];
    _pk1_jacobian[_qp] = _C[_qp];
  }
}
