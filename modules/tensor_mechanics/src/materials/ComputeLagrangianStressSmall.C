#include "ComputeLagrangianStressSmall.h"

InputParameters
ComputeLagrangianStressSmall::validParams()
{
  InputParameters params = ComputeLagrangianStressCauchy::validParams();

  MooseEnum objectiveRate("truesdell jaumann", "truesdell");
  params.addParam<MooseEnum>(
      "objective_rate", objectiveRate, "Which type of objective integration to use");

  return params;
}

ComputeLagrangianStressSmall::ComputeLagrangianStressSmall(const InputParameters & parameters)
  : ComputeLagrangianStressCauchy(parameters),
    _small_stress(declareProperty<RankTwoTensor>("small_stress")),
    _small_stress_old(getMaterialPropertyOld<RankTwoTensor>("small_stress")),
    _small_jacobian(declareProperty<RankFourTensor>("small_jacobian")),
    _cauchy_stress_old(getMaterialPropertyOld<RankTwoTensor>("stress")),
    _mechanical_strain(getMaterialPropertyByName<RankTwoTensor>("mechanical_strain")),
    _strain_increment(getMaterialPropertyByName<RankTwoTensor>("strain_increment")),
    _rate(getParam<MooseEnum>("objective_rate").getEnum<ObjectiveRate>())
{
}

void
ComputeLagrangianStressSmall::computeQpCauchyStress()
{
  computeQpSmallStress();
  // Actually do the objective integration
  if (_ld)
  {
    _objectiveUpdate();
  }
  // Just a copy for small strains
  else
  {
    _cauchy_stress[_qp] = _small_stress[_qp];
    _cauchy_jacobian[_qp] = _small_jacobian[_qp];
  }
}

void
ComputeLagrangianStressSmall::_objectiveUpdate()
{
  // Common to most/all models

  // Small stress increment (really this all that needs to be defined)
  RankTwoTensor dS = _small_stress[_qp] - _small_stress_old[_qp];
  // Increment in the spatial velocity gradient
  RankTwoTensor dL = RankTwoTensor::Identity() - _inv_df[_qp];

  // Get the appropriate update tensor
  RankFourTensor J;
  switch (_rate)
  {
    case ObjectiveRate::Truesdell:
      J = _updateTensor(dL);
      break;
    case ObjectiveRate::Jaumann:
      J = _updateTensor(0.5 * (dL - dL.transpose()));
      break;
  }

  // Update the Cauchy stress
  RankFourTensor Jinv = J.inverse();
  _cauchy_stress[_qp] = Jinv * (_cauchy_stress_old[_qp] + dS);

  // Get the appropriate tangent tensor
  RankFourTensor U;
  switch (_rate)
  {
    case ObjectiveRate::Truesdell:
      U = _truesdellTangent(_cauchy_stress[_qp]);
      break;
    case ObjectiveRate::Jaumann:
      U = _jaumannTangent(_cauchy_stress[_qp]);
      break;
  }

  // Update the tangent
  auto Isym = RankFourTensor(RankFourTensor::initIdentitySymmetricFour);
  _cauchy_jacobian[_qp] = Jinv * (_small_jacobian[_qp] * Isym - U);
}

RankFourTensor
ComputeLagrangianStressSmall::_updateTensor(const RankTwoTensor & Q)
{
  RankFourTensor J;
  auto I = RankTwoTensor::Identity();
  auto trQ = Q.trace();

  for (size_t i = 0; i < 3; i++)
  {
    for (size_t j = 0; j < 3; j++)
    {
      for (size_t m = 0; m < 3; m++)
      {
        for (size_t n = 0; n < 3; n++)
        {
          J(i, j, m, n) = (1.0 + trQ) * I(i, m) * I(j, n) - Q(i, m) * I(j, n) - I(i, m) * Q(j, n);
        }
      }
    }
  }
  return J;
}

RankFourTensor
ComputeLagrangianStressSmall::_truesdellTangent(const RankTwoTensor & S)
{
  RankFourTensor U;
  auto I = RankTwoTensor::Identity();

  for (size_t m = 0; m < 3; m++)
  {
    for (size_t n = 0; n < 3; n++)
    {
      for (size_t k = 0; k < 3; k++)
      {
        for (size_t l = 0; l < 3; l++)
        {
          U(m, n, k, l) = I(k, l) * S(m, n) - I(m, k) * S(l, n) - I(n, k) * S(m, l);
        }
      }
    }
  }

  return U;
}

RankFourTensor
ComputeLagrangianStressSmall::_jaumannTangent(const RankTwoTensor & S)
{
  RankFourTensor U;
  auto I = RankTwoTensor::Identity();

  for (size_t m = 0; m < 3; m++)
  {
    for (size_t n = 0; n < 3; n++)
    {
      for (size_t k = 0; k < 3; k++)
      {
        for (size_t l = 0; l < 3; l++)
        {
          U(m, n, k, l) =
              0.5 * (I(m, l) * S(k, n) + I(n, l) * S(m, k) - I(m, k) * S(l, n) - I(n, k) * S(m, l));
        }
      }
    }
  }

  return U;
}
