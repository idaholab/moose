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
    _deformation_gradient_increment(getMaterialPropertyByName<RankTwoTensor>(
        _base_name + "spatial_deformation_gradient_increment")),
    _d_deformation_gradient_increment_d_F(getMaterialPropertyByName<RankFourTensor>(
        _base_name + "d_spatial_deformation_gradient_increment_d_deformation_gradient")),
    _d_vorticity_increment_d_F(getMaterialPropertyByName<RankFourTensor>(
        _base_name + "d_vorticity_increment_d_deformation_gradient")),
    _def_grad(getMaterialPropertyByName<RankTwoTensor>(_base_name + "deformation_gradient")),
    _def_grad_old(getMaterialPropertyOldByName<RankTwoTensor>(_base_name + "deformation_gradient")),
    _objective_rate(getParam<MooseEnum>("objective_rate")),
    _rotate_old_stress(getParam<bool>("rotate_old_stress"))
{
  // Only the Green-Naghdi rate consumes the polar-decomposition rotation. Fetch these
  // (which marks `rotation` active and triggers the strain calc's polar decomposition) only
  // for that rate -- Truesdell/Jaumann/Rashid leave them null and skip the eigensolve.
  if (_objective_rate == "green_naghdi")
  {
    _rotation = &getMaterialPropertyByName<RankTwoTensor>(_base_name + "rotation");
    _rotation_old = &getMaterialPropertyOldByName<RankTwoTensor>(_base_name + "rotation");
    _d_rotation_d_F = &getMaterialPropertyByName<RankFourTensor>(
        _base_name + "d_rotation_d_deformation_gradient");
  }
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

  const bool need_jacobian = _fe_problem.currentlyComputingJacobian() ||
                             _fe_problem.currentlyComputingResidualAndJacobian();

  if (!_large_kinematics)
  {
    _cauchy_stress[_qp] = _small_stress[_qp];
    // Small kinematics: no objective advection. dsigma/d_eigenstrain = -small_jacobian
    // (the negative because mechanical_strain = total_strain - eigenstrain).
    // Both Jacobian-side assignments are skipped on residual-only sweeps; the kernel
    // consumes neither during residual assembly.
    if (need_jacobian)
    {
      _cauchy_jacobian[_qp] = _small_jacobian[_qp];
      _dcauchy_stress_d_eigenstrain[_qp] = -_small_jacobian[_qp];
    }
  }
  else
  {
    const RankTwoTensor dS = _small_stress[_qp] - _small_stress_old[_qp];

    // Gather the per-qp quantities the rate needs, dispatch to the selected objective rate, and
    // scatter the outputs back into our properties.
    LagrangianObjectiveRates::Inputs in;
    in.dS = dS;
    in.cauchy_stress_old = _cauchy_stress_old[_qp];
    in.small_jacobian = _small_jacobian[_qp];
    in.dL = _deformation_gradient_increment[_qp];
    in.dW = _vorticity_increment[_qp];
    in.d_dL_d_F = _d_deformation_gradient_increment_d_F[_qp];
    in.d_dW_d_F = _d_vorticity_increment_d_F[_qp];
    // Green-Naghdi only: the rotation properties (and inv_df / F^{-1}) are fetched only for that
    // rate, so `_rotation` is non-null exactly when `greenNaghdi` will read them.
    if (_rotation)
    {
      in.rotation = (*_rotation)[_qp];
      in.rotation_old = (*_rotation_old)[_qp];
      in.d_rotation_d_F = (*_d_rotation_d_F)[_qp];
      in.inv_df = _inv_df[_qp];
      in.inv_def_grad = _inv_def_grad[_qp];
    }

    const auto out = LagrangianObjectiveRates::compute(_objective_rate, in, need_jacobian);
    _cauchy_stress[_qp] = out.cauchy_stress;
    if (need_jacobian)
    {
      _cauchy_jacobian[_qp] = out.cauchy_jacobian;
      _dcauchy_stress_d_eigenstrain[_qp] = out.dcauchy_stress_d_eigenstrain;
    }

    if (_rotate_old_stress)
    {
      // Passthrough mode: the wrapped stress material (e.g. `ComputeMultiPlasticityStress`
      // with `perform_finite_strain_rotations = true`, fed our published
      // `_rotation_increment`) has already produced the correctly-rotated cumulative Cauchy
      // stress. Use `_small_stress` directly and discard the rate's own outer rotation of
      // the stress. `_cauchy_jacobian` from the rate's chain rule is still correct because
      // it computes `d sigma_cauchy / d(dL)` from `_small_jacobian = dDeltasigma_const/deps` plus
      // the same rotation-derivative terms -- algebraically the same chain whether we view sigma as
      // produced by the rate's `R * (sigma_old + Deltasigma_const) * R^T` formula or by the wrapped
      // material's `R * (s_old + Deltasigma_const) * R^T = small_stress` storage.
      _cauchy_stress[_qp] = _small_stress[_qp];
    }
  }
}
