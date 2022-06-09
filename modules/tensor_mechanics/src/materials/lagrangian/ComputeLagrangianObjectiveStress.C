//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLagrangianObjectiveStress.h"

InputParameters
ComputeLagrangianObjectiveStress::validParams()
{
  InputParameters params = ComputeLagrangianStressCauchy::validParams();

  params.addClassDescription("Stress update based on the small (engineering) stress");

  MooseEnum objectiveRate("truesdell jaumann", "truesdell");
  params.addParam<MooseEnum>(
      "objective_rate", objectiveRate, "Which type of objective integration to use");

  return params;
}

ComputeLagrangianObjectiveStress::ComputeLagrangianObjectiveStress(
    const InputParameters & parameters)
  : ComputeLagrangianStressCauchy(parameters),
    _small_stress(declareProperty<RankTwoTensor>(_base_name + "small_stress")),
    _small_stress_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "small_stress")),
    _small_jacobian(declareProperty<RankFourTensor>(_base_name + "small_jacobian")),
    _cauchy_stress_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "cauchy_stress")),
    _mechanical_strain(getMaterialPropertyByName<RankTwoTensor>(_base_name + "mechanical_strain")),
    _strain_increment(getMaterialPropertyByName<RankTwoTensor>(_base_name + "strain_increment")),
    _rate(getParam<MooseEnum>("objective_rate").getEnum<ObjectiveRate>())
{
}

void
ComputeLagrangianObjectiveStress::initQpStatefulProperties()
{
  ComputeLagrangianStressBase::initQpStatefulProperties();

  _small_stress[_qp].zero();
  _cauchy_stress[_qp].zero();
}

void
ComputeLagrangianObjectiveStress::computeQpCauchyStress()
{
  computeQpSmallStress();
  // Actually do the objective integration
  if (_large_kinematics)
  {
    computeQpObjectiveUpdate();
  }
  // Just a copy for small strains
  else
  {
    _cauchy_stress[_qp] = _small_stress[_qp];
    _cauchy_jacobian[_qp] = _small_jacobian[_qp];
  }
}

void
ComputeLagrangianObjectiveStress::computeQpObjectiveUpdate()
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
      J = updateTensor(dL);
      break;
    case ObjectiveRate::Jaumann:
      J = updateTensor(0.5 * (dL - dL.transpose()));
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
      U = truesdellTangent(_cauchy_stress[_qp]);
      break;
    case ObjectiveRate::Jaumann:
      U = jaumannTangent(_cauchy_stress[_qp]);
      break;
  }

  // Update the tangent
  auto Isym = RankFourTensor(RankFourTensor::initIdentitySymmetricFour);
  _cauchy_jacobian[_qp] = Jinv * (_small_jacobian[_qp] * Isym - U);
}

RankFourTensor
ComputeLagrangianObjectiveStress::updateTensor(const RankTwoTensor & Q)
{
  auto I = RankTwoTensor::Identity();
  usingTensorIndices(i, j, k, l);
  return (1.0 + Q.trace()) * I.times<i, k, j, l>(I) - Q.times<i, k, j, l>(I) -
         I.times<i, k, j, l>(Q);
}

RankFourTensor
ComputeLagrangianObjectiveStress::truesdellTangent(const RankTwoTensor & S)
{
  auto I = RankTwoTensor::Identity();
  usingTensorIndices(i, j, k, l);
  return S.times<i, j, k, l>(I) - I.times<i, k, j, l>(S.transpose()) - S.times<i, l, j, k>(I);
}

RankFourTensor
ComputeLagrangianObjectiveStress::jaumannTangent(const RankTwoTensor & S)
{
  auto I = RankTwoTensor::Identity();
  usingTensorIndices(i, j, k, l);
  return 0.5 * (I.times<i, l, j, k>(S.transpose()) + S.times<i, k, j, l>(I) -
                I.times<i, k, j, l>(S.transpose()) - S.times<i, l, j, k>(I));
}
