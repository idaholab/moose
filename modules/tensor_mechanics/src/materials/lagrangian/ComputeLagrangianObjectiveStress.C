//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLagrangianObjectiveStress.h"

#include "FactorizedRankTwoTensor.h"

InputParameters
ComputeLagrangianObjectiveStress::validParams()
{
  InputParameters params = ComputeLagrangianStressCauchy::validParams();

  params.addClassDescription("Stress update based on the small (engineering) stress");

  MooseEnum objectiveRate("truesdell jaumann green_naghdi", "truesdell");
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
    _vorticity_increment(
        getMaterialPropertyByName<RankTwoTensor>(_base_name + "vorticity_increment")),
    _def_grad(getMaterialPropertyByName<RankTwoTensor>(_base_name + "deformation_gradient")),
    _def_grad_old(getMaterialPropertyOldByName<RankTwoTensor>(_base_name + "deformation_gradient")),
    _rate(getParam<MooseEnum>("objective_rate").getEnum<ObjectiveRate>()),
    _polar_decomp(_rate == ObjectiveRate::GreenNaghdi),
    _rotation(_polar_decomp ? &declareProperty<RankTwoTensor>(_base_name + "rotation") : nullptr),
    _rotation_old(_polar_decomp ? &getMaterialPropertyOld<RankTwoTensor>(_base_name + "rotation")
                                : nullptr),
    _d_rotation_d_def_grad(
        _polar_decomp ? &declareProperty<RankFourTensor>(derivativePropertyName(
                            _base_name + "rotation", {_base_name + "deformation_gradient"}))
                      : nullptr),
    _stretch(_polar_decomp ? &declareProperty<RankTwoTensor>(_base_name + "stretch") : nullptr)
{
}

void
ComputeLagrangianObjectiveStress::initQpStatefulProperties()
{
  ComputeLagrangianStressBase::initQpStatefulProperties();

  _small_stress[_qp].zero();
  _cauchy_stress[_qp].zero();

  if (_polar_decomp)
    (*_rotation)[_qp] = RankTwoTensor::Identity();
}

void
ComputeLagrangianObjectiveStress::computeQpCauchyStress()
{
  computeQpSmallStress();

  if (!_large_kinematics)
  {
    _cauchy_stress[_qp] = _small_stress[_qp];
    _cauchy_jacobian[_qp] = _small_jacobian[_qp];
  }
  else
  {
    // If large_kinematics = true, do the objective integration
    RankTwoTensor dS = _small_stress[_qp] - _small_stress_old[_qp];

    if (_rate == ObjectiveRate::Truesdell)
      _cauchy_stress[_qp] = objectiveUpdateTruesdell(dS);
    else if (_rate == ObjectiveRate::Jaumann)
      _cauchy_stress[_qp] = objectiveUpdateJaumann(dS);
    else if (_rate == ObjectiveRate::GreenNaghdi)
      _cauchy_stress[_qp] = objectiveUpdateGreenNaghdi(dS);
    else
      mooseError("Internal error: unsupported objective rate.");
  }
}

RankTwoTensor
ComputeLagrangianObjectiveStress::objectiveUpdateTruesdell(const RankTwoTensor & dS)
{
  // Get the kinematic tensor
  RankTwoTensor dL = RankTwoTensor::Identity() - _inv_df[_qp];

  // Update the Cauchy stress
  auto [S, Jinv] = advectStress(_cauchy_stress_old[_qp] + dS, dL);

  // Get the appropriate tangent tensor
  RankFourTensor U = stressAdvectionDerivative(S);
  _cauchy_jacobian[_qp] = cauchyJacobian(Jinv, U);

  return S;
}

RankTwoTensor
ComputeLagrangianObjectiveStress::objectiveUpdateJaumann(const RankTwoTensor & dS)
{
  usingTensorIndices(i, j, k, l);

  // Get the kinematic tensor
  RankTwoTensor dL = RankTwoTensor::Identity() - _inv_df[_qp];
  RankTwoTensor dW = 0.5 * (dL - dL.transpose());

  // Update the Cauchy stress
  auto [S, Jinv] = advectStress(_cauchy_stress_old[_qp] + dS, dW);

  // Get the appropriate tangent tensor
  RankTwoTensor I = RankTwoTensor::Identity();
  RankFourTensor ddW_ddL = 0.5 * (I.times<i, k, j, l>(I) - I.times<i, l, j, k>(I));
  RankFourTensor U = stressAdvectionDerivative(S) * ddW_ddL;
  _cauchy_jacobian[_qp] = cauchyJacobian(Jinv, U);

  return S;
}

RankTwoTensor
ComputeLagrangianObjectiveStress::objectiveUpdateGreenNaghdi(const RankTwoTensor & dS)
{
  usingTensorIndices(i, j, k, l, m);

  // The kinematic tensor for the Green-Naghdi rate is
  // Omega = dot(R) R^T
  polarDecomposition();
  RankTwoTensor I = RankTwoTensor::Identity();
  RankTwoTensor dR = (*_rotation)[_qp] * (*_rotation_old)[_qp].transpose() - I;
  RankTwoTensor dO = dR * _inv_df[_qp];

  // Update the Cauchy stress
  auto [S, Jinv] = advectStress(_cauchy_stress_old[_qp] + dS, dO);

  // Get the appropriate tangent tensor
  RankFourTensor d_R_d_F = (*_d_rotation_d_def_grad)[_qp];
  RankFourTensor d_F_d_dL = _inv_df[_qp].inverse().times<i, k, l, j>(_def_grad[_qp]);
  RankTwoTensor T = (*_rotation_old)[_qp].transpose() * _inv_df[_qp];
  RankFourTensor d_dO_d_dL =
      T.times<m, j, i, m, k, l>(d_R_d_F * d_F_d_dL) - dR.times<i, k, j, l>(I);
  RankFourTensor U = stressAdvectionDerivative(S) * d_dO_d_dL;
  _cauchy_jacobian[_qp] = cauchyJacobian(Jinv, U);

  return S;
}

std::tuple<RankTwoTensor, RankFourTensor>
ComputeLagrangianObjectiveStress::advectStress(const RankTwoTensor & S0,
                                               const RankTwoTensor & dQ) const
{
  RankFourTensor J = updateTensor(dQ);
  RankFourTensor Jinv = J.inverse();
  RankTwoTensor S = Jinv * S0;
  return {S, Jinv};
}

RankFourTensor
ComputeLagrangianObjectiveStress::updateTensor(const RankTwoTensor & dQ) const
{
  auto I = RankTwoTensor::Identity();
  usingTensorIndices(i, j, k, l);
  return (1.0 + dQ.trace()) * I.times<i, k, j, l>(I) - dQ.times<i, k, j, l>(I) -
         I.times<i, k, j, l>(dQ);
}

RankFourTensor
ComputeLagrangianObjectiveStress::stressAdvectionDerivative(const RankTwoTensor & S) const
{
  auto I = RankTwoTensor::Identity();
  usingTensorIndices(i, j, k, l);
  return S.times<i, j, k, l>(I) - I.times<i, k, l, j>(S) - S.times<i, l, j, k>(I);
}

RankFourTensor
ComputeLagrangianObjectiveStress::cauchyJacobian(const RankFourTensor & Jinv,
                                                 const RankFourTensor & U) const
{
  return Jinv * (_small_jacobian[_qp] - U);
}

void
ComputeLagrangianObjectiveStress::polarDecomposition()
{
  FactorizedRankTwoTensor C = _def_grad[_qp].transpose() * _def_grad[_qp];
  (*_stretch)[_qp] = MathUtils::sqrt(C).get();
  RankTwoTensor Uinv = MathUtils::sqrt(C).inverse().get();
  (*_rotation)[_qp] = _def_grad[_qp] * Uinv;

  // Derivative of rotation w.r.t. the deformation gradient
  RankTwoTensor I = RankTwoTensor::Identity();
  RankTwoTensor Y = (*_stretch)[_qp].trace() * I - (*_stretch)[_qp];
  RankTwoTensor Z = (*_rotation)[_qp] * Y;
  RankTwoTensor O = Z * (*_rotation)[_qp].transpose();
  usingTensorIndices(i, j, k, l);
  (*_d_rotation_d_def_grad)[_qp] = (O.times<i, k, l, j>(Y) - Z.times<i, l, k, j>(Z)) / Y.det();
}
