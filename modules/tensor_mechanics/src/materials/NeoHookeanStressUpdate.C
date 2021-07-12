#include "NeoHookeanStressUpdate.h"

registerMooseObject("TensorMechanicsApp", NeoHookeanStressUpdate);

InputParameters
NeoHookeanStressUpdate::validParams()
{
  InputParameters params = ComputeLagrangianStressPK2::validParams();

  params.addParam<MaterialPropertyName>("lambda",
                                        "lambda",
                                        "Parameter conjugate to Lame parameter"
                                        " for small deformations");
  params.addParam<MaterialPropertyName>("mu",
                                        "mu",
                                        "Parameter conjugate to Lame parameter"
                                        " for small deformations");

  return params;
}

NeoHookeanStressUpdate::NeoHookeanStressUpdate(const InputParameters & parameters)
  : ComputeLagrangianStressPK2(parameters),
    _lambda(getMaterialProperty<Real>(getParam<MaterialPropertyName>("lambda"))),
    _mu(getMaterialProperty<Real>(getParam<MaterialPropertyName>("mu")))

{
}

void
NeoHookeanStressUpdate::computeQpPK2Stress()
{
  // Hyperelasticity is weird, we need to branch on the type of update if we
  // want a truly linear model
  //
  // This is because we need to drop quadratic terms for the linear update

  // Large deformation = nonlinear strain
  if (_large_kinematics)
  {
    RankTwoTensor Cinv = (2 * _E[_qp] + RankTwoTensor::Identity()).inverse();
    _S[_qp] =
        (_lambda[_qp] * log(_detJ[_qp]) - _mu[_qp]) * Cinv + _mu[_qp] * RankTwoTensor::Identity();
    _C[_qp] =
        -2 * (_lambda[_qp] * log(_detJ[_qp]) - _mu[_qp]) * Cinv.mixedProductIkJl(Cinv.transpose()) +
        _lambda[_qp] * Cinv.outerProduct(Cinv);
  }
  // Small deformations = linear strain
  else
  {
    RankTwoTensor strain = 0.5 * (_F[_qp] + _F[_qp].transpose());
    RankTwoTensor I = RankTwoTensor::Identity();
    _C[_qp] = _lambda[_qp] * I.outerProduct(I) +
              2 * _mu[_qp] * RankFourTensor(RankFourTensor::initIdentitySymmetricFour);
    _S[_qp] = _C[_qp] * strain;
  }
}
