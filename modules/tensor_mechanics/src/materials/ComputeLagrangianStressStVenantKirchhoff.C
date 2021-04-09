#include "ComputeLagrangianStressStVenantKirchhoff.h"

registerMooseObject("TensorMechanicsApp", ComputeLagrangianStressStVenantKirchhoff);

InputParameters
ComputeLagrangianStressStVenantKirchhoff::validParams()
{
  InputParameters params = ComputeLagrangianStressPK::validParams();

  params.addParam<Real>("mu", "Lame parameter mu");
  params.addParam<Real>("lambda", "Lame parameter lambda");

  return params;
}

ComputeLagrangianStressStVenantKirchhoff::ComputeLagrangianStressStVenantKirchhoff(
    const InputParameters & parameters)
  : ComputeLagrangianStressPK(parameters),
    _mu(getParam<Real>("mu")),
    _lambda(getParam<Real>("lambda"))
{
}

void
ComputeLagrangianStressStVenantKirchhoff::computeQpPKStress()
{
  // Hyperelasticity is weird, we need to branch on the type of update if we
  // want a truly linear model
  //
  // Model with nonlinear strain
  if (_ld)
  {
    RankTwoTensor E = 0.5 * (_F[_qp].transpose() * _F[_qp] - RankTwoTensor::Identity());
    auto S = _lambda * E.trace() * RankTwoTensor::Identity() + 2 * _mu * E;
    _pk1_stress[_qp] = _F[_qp] * S;

    // Identity and extra tensors
    RankTwoTensor I = RankTwoTensor::Identity();
    RankFourTensor G;
    RankFourTensor II;
    RankFourTensor EE;
    for (size_t i = 0; i < 3; i++)
    {
      for (size_t j = 0; j < 3; j++)
      {
        for (size_t k = 0; k < 3; k++)
        {
          for (size_t l = 0; l < 3; l++)
          {
            II(i, j, k, l) = I(i, j) * I(k, l);
            G(i, j, k, l) = I(i, k) * S(l, j);
            EE(i, j, k, l) = 0.5 * (I(i, l) * _F[_qp](k, j) + _F[_qp](k, i) * I(j, l));
          }
        }
      }
    }

    RankFourTensor C =
        _lambda * II + 2.0 * _mu * RankFourTensor(RankFourTensor::initIdentitySymmetricFour);
    RankFourTensor CE = C * EE;
    _pk1_jacobian[_qp] = G;
    for (size_t i = 0; i < 3; i++)
      for (size_t j = 0; j < 3; j++)
        for (size_t k = 0; k < 3; k++)
          for (size_t l = 0; l < 3; l++)
            for (size_t m = 0; m < 3; m++)
              _pk1_jacobian[_qp](i, j, k, l) += _F[_qp](i, m) * CE(m, j, k, l);
  }
  // Model with linear strain
  else
  {
    RankTwoTensor I = RankTwoTensor::Identity();
    RankFourTensor II;
    // Identity thingy
    for (size_t i = 0; i < 3; i++)
    {
      for (size_t j = 0; j < 3; j++)
      {
        for (size_t k = 0; k < 3; k++)
        {
          for (size_t l = 0; l < 3; l++)
          {
            II(i, j, k, l) = I(i, j) * I(k, l);
          }
        }
      }
    }

    RankTwoTensor E = 0.5 * (_F[_qp] + _F[_qp].transpose());
    _pk1_stress[_qp] = _lambda * E.trace() * RankTwoTensor::Identity() + 2 * _mu * E;
    _pk1_jacobian[_qp] =
        _lambda * II + 2 * _mu * RankFourTensor(RankFourTensor::initIdentitySymmetricFour);
  }
}
