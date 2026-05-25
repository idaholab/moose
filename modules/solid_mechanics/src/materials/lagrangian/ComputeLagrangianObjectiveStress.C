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

  MooseEnum objectiveRate("truesdell jaumann green_naghdi rashid", "truesdell");
  params.addParam<MooseEnum>(
      "objective_rate", objectiveRate, "Which type of objective integration to use");
  params.addParam<bool>(
      "rotate_old_stress",
      false,
      "If true, the rate runs in passthrough mode: `cauchy_stress = small_stress` (no outer "
      "rotation applied). Use when wrapping a material with `perform_finite_strain_rotations = "
      "true` whose own internal rotation already produces the correct cumulative Cauchy stress "
      "(needs the strain calculator to publish a non-identity `rotation_increment` via "
      "`publish_rotation_increment = true`). `cauchy_jacobian` is still computed via the rate's "
      "chain rule. Default false preserves the standard objective-rate pipeline.");

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
    _rate_strategy(createObjectiveRate(getParam<MooseEnum>("objective_rate"))),
    _rotate_old_stress(getParam<bool>("rotate_old_stress"))
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
    const RankTwoTensor dS = _small_stress[_qp] - _small_stress_old[_qp];
    _rate_strategy->update(*this, dS);
    if (_rotate_old_stress)
    {
      // Passthrough mode: the wrapped stress material (e.g. `ComputeMultiPlasticityStress`
      // with `perform_finite_strain_rotations = true`, fed our published
      // `_rotation_increment`) has already produced the correctly-rotated cumulative Cauchy
      // stress. Use `_small_stress` directly and discard the rate's own outer rotation of
      // the stress. `_cauchy_jacobian` from the rate's chain rule is still correct because
      // it computes `d σ_cauchy / d(dL)` from `_small_jacobian = dΔσ_const/dε` plus the
      // same rotation-derivative terms — algebraically the same chain whether we view σ as
      // produced by the rate's `R · (σ_old + Δσ_const) · R^T` formula or by the wrapped
      // material's `R · (s_old + Δσ_const) · R^T = small_stress` storage.
      _cauchy_stress[_qp] = _small_stress[_qp];
    }
  }
}
