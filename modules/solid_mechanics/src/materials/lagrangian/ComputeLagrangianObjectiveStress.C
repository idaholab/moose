//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
    _dcauchy_stress_d_eigenstrain(
        declareProperty<RankFourTensor>(_base_name + "dcauchy_stress_d_eigenstrain")),
    _cauchy_stress_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "cauchy_stress")),
    _mechanical_strain(getMaterialPropertyByName<RankTwoTensor>(_base_name + "mechanical_strain")),
    _strain_increment(getMaterialPropertyByName<RankTwoTensor>(_base_name + "strain_increment")),
    _vorticity_increment(
        getMaterialPropertyByName<RankTwoTensor>(_base_name + "vorticity_increment")),
    _spatial_velocity_increment(
        getMaterialPropertyByName<RankTwoTensor>(_base_name + "spatial_velocity_increment")),
    _d_spatial_velocity_increment_d_F(getMaterialPropertyByName<RankFourTensor>(
        _base_name + "d_spatial_velocity_increment_d_deformation_gradient")),
    _d_vorticity_increment_d_F(getMaterialPropertyByName<RankFourTensor>(
        _base_name + "d_vorticity_increment_d_deformation_gradient")),
    _rotation(getMaterialPropertyByName<RankTwoTensor>(_base_name + "rotation")),
    _rotation_old(getMaterialPropertyOldByName<RankTwoTensor>(_base_name + "rotation")),
    _d_rotation_d_F(getMaterialPropertyByName<RankFourTensor>(
        _base_name + "d_rotation_d_deformation_gradient")),
    _def_grad(getMaterialPropertyByName<RankTwoTensor>(_base_name + "deformation_gradient")),
    _def_grad_old(getMaterialPropertyOldByName<RankTwoTensor>(_base_name + "deformation_gradient")),
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

  if (!_large_kinematics)
  {
    _cauchy_stress[_qp] = _small_stress[_qp];
    _cauchy_jacobian[_qp] = _small_jacobian[_qp];
    // Small kinematics: no objective advection. dsigma/d_eigenstrain = -small_jacobian
    // (the negative because mechanical_strain = total_strain - eigenstrain).
    _dcauchy_stress_d_eigenstrain[_qp] = -_small_jacobian[_qp];
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
  // Use the stored full kinematic spatial velocity gradient increment.
  const RankTwoTensor & dL = _spatial_velocity_increment[_qp];

  // Update the Cauchy stress
  auto [S, Jinv] = advectStress(_cauchy_stress_old[_qp] + dS, dL);
  // d_sigma/d_eigenstrain = -Jinv * small_jacobian. The negative sign comes from
  // mechanical_strain = total_strain - eigenstrain (an eigenstrain increase reduces
  // the mechanical strain that drives small_stress).
  _dcauchy_stress_d_eigenstrain[_qp] = -Jinv * _small_jacobian[_qp];

  // Get the appropriate tangent tensor
  RankFourTensor U = stressAdvectionDerivative(S);
  _cauchy_jacobian[_qp] = cauchyJacobian(Jinv, U);

  return S;
}

RankTwoTensor
ComputeLagrangianObjectiveStress::objectiveUpdateJaumann(const RankTwoTensor & dS)
{
  // The kinematic tensor for Jaumann is the vorticity increment ΔW, which the strain
  // calculator publishes for whichever kinematic_approximation the user selected.
  const RankTwoTensor & dW = _vorticity_increment[_qp];

  // Update the Cauchy stress
  auto [S, Jinv] = advectStress(_cauchy_stress_old[_qp] + dS, dW);
  // d_sigma/d_eigenstrain = -Jinv * small_jacobian. The negative sign comes from
  // mechanical_strain = total_strain - eigenstrain (an eigenstrain increase reduces
  // the mechanical strain that drives small_stress).
  _dcauchy_stress_d_eigenstrain[_qp] = -Jinv * _small_jacobian[_qp];

  // d(dW)/d(dL) by chain rule: d(dW)/d(F) · (d(dL)/d(F))^{-1}. For linear this collapses
  // to the bare skew projector 0.5(I^{ikjl} - I^{iljk}) that the old code hard-coded.
  const RankFourTensor d_dW_d_dL =
      _d_vorticity_increment_d_F[_qp] * _d_spatial_velocity_increment_d_F[_qp].inverse();
  const RankFourTensor U = stressAdvectionDerivative(S) * d_dW_d_dL;
  _cauchy_jacobian[_qp] = cauchyJacobian(Jinv, U);

  return S;
}

RankTwoTensor
ComputeLagrangianObjectiveStress::objectiveUpdateGreenNaghdi(const RankTwoTensor & dS)
{
  usingTensorIndices(i, j, k, l, m);

  // The kinematic tensor for the Green-Naghdi rate is dot(R) R^T. R, R_old, and dR/dF
  // come from the strain calculator (polar decomposition of F_actual at n+1).
  const RankTwoTensor I = RankTwoTensor::Identity();
  const RankTwoTensor dR = _rotation[_qp] * _rotation_old[_qp].transpose() - I;
  const RankTwoTensor dO = dR * _inv_df[_qp];

  // Update the Cauchy stress
  auto [S, Jinv] = advectStress(_cauchy_stress_old[_qp] + dS, dO);
  _dcauchy_stress_d_eigenstrain[_qp] = -Jinv * _small_jacobian[_qp];

  // Tangent: chain d(dO)/d(F) -> d(F)/d(dL). dO depends on F through R = polar(F) and
  // through inv_df = F_old · F^{-1}; both contributions need to flow through the strain
  // calculator's d(dL)/d(F) so the chain stays consistent across all
  // kinematic_approximation options.
  const RankFourTensor & d_R_d_F = _d_rotation_d_F[_qp];
  const RankFourTensor d_F_d_dL = _d_spatial_velocity_increment_d_F[_qp].inverse();
  const RankTwoTensor T = _rotation_old[_qp].transpose() * _inv_df[_qp];

  // d(inv_df)/d(F) = -inv_df ⊗ F^{-T} (from d(F^{-1})/dF = -F^{-1} ⊗ F^{-T} and
  // inv_df = F_old · F^{-1}). For the linear kinematic approximation this combines
  // with the d_F_d_dL inverse to yield -I^(4), recovering the old
  // "-dR.times<i,k,j,l>(I)" shortcut byte-for-byte. For non-linear options the chain
  // through the strain calculator's d(dL)/dF stays consistent.
  const RankFourTensor d_invdf_d_F =
      -_inv_df[_qp].times<i, k, l, j>(_inv_def_grad[_qp]);
  const RankFourTensor d_invdf_d_dL = d_invdf_d_F * d_F_d_dL;
  // d(dO)/d(inv_df) = dR_ip * δ_jq, written as dR.times<i, k, j, l>(I) (output ijkl with
  // (k, l) as the inv_df indices).
  const RankFourTensor d_dO_d_invdf = dR.times<i, k, j, l>(I);

  const RankFourTensor d_dO_d_dL =
      T.times<m, j, i, m, k, l>(d_R_d_F * d_F_d_dL) + d_dO_d_invdf * d_invdf_d_dL;
  const RankFourTensor U = stressAdvectionDerivative(S) * d_dO_d_dL;
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

