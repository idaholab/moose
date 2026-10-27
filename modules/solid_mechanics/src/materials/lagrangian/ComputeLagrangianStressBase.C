//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLagrangianStressBase.h"

InputParameters
ComputeLagrangianStressBase::validParams()
{
  InputParameters params = Material::validParams();

  params.addDeprecatedParam<bool>(
      "large_kinematics",
      false,
      "Use a large displacement stress update.",
      "large_kinematics is no longer set on the stress calculator; it is derived from the "
      "ComputeLagrangianStrain calculator (the single source of truth) via the LARGE_KINEMATICS "
      "guarantee. Remove it here and set it only on the strain calculator.");

  params.addParam<std::string>("base_name", "Material property base name");

  return params;
}

ComputeLagrangianStressBase::ComputeLagrangianStressBase(const InputParameters & parameters)
  : Material(parameters),
    GuaranteeConsumer(this),
    // Derived from the strain calculator's guarantee in initialSetup(); the local parameter is
    // deprecated and only consulted for a consistency cross-check.
    _large_kinematics(false),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _cauchy_stress(declareProperty<RankTwoTensor>(_base_name + "cauchy_stress")),
    _cauchy_jacobian(declareProperty<RankFourTensor>(_base_name + "cauchy_jacobian")),
    _pk1_stress(declareProperty<RankTwoTensor>(_base_name + "pk1_stress")),
    _pk1_jacobian(declareProperty<RankFourTensor>(_base_name + "pk1_jacobian")),
    _pk1_jacobian_bypass_fbar(
        declareProperty<RankFourTensor>(_base_name + "pk1_jacobian_bypass_fbar")),
    _dpk1_d_grad_u(declareProperty<RankFourTensor>(_base_name + "dpk1_d_grad_u")),
    _d_F_d_grad_u(getMaterialPropertyByName<RankFourTensor>(
        _base_name + "d_deformation_gradient_d_grad_displacement")),
    _F_ust(
        getMaterialPropertyByName<RankTwoTensor>(_base_name + "unstabilized_deformation_gradient")),
    _d_deformation_gradient_increment_d_F(getMaterialPropertyByName<RankFourTensor>(
        _base_name + "d_spatial_deformation_gradient_increment_d_deformation_gradient")),
    _d_F_stab_d_F_ust(
        getMaterialPropertyByName<RankFourTensor>(_base_name + "d_F_stab_d_F_unstabilized")),
    _d_F_stab_d_F_avg(
        getMaterialPropertyByName<RankFourTensor>(_base_name + "d_F_stab_d_F_average")),
    _d_nl_fbar(declareProperty<RankFourTensor>(_base_name + "d_nl_fbar_operator"))
{
}

void
ComputeLagrangianStressBase::initialSetup()
{
  Material::initialSetup();

  // Derive the kinematics regime from the strain calculator's LARGE_KINEMATICS guarantee -- the
  // single source of truth. Block-restricted (per subdomain) and keyed by the base_name-prefixed
  // deformation_gradient (so a given base_name matches its own strain calculator).
  _large_kinematics = hasGuaranteedMaterialProperty(_base_name + "deformation_gradient",
                                                    Guarantee::LARGE_KINEMATICS);

  // The deprecated local parameter must not silently disagree with the strain calculator.
  if (isParamSetByUser("large_kinematics") &&
      getParam<bool>("large_kinematics") != _large_kinematics)
    paramError("large_kinematics",
               "large_kinematics disagrees with the ComputeLagrangianStrain calculator (which "
               "computes ",
               _large_kinematics ? "large" : "small",
               " kinematics). large_kinematics is deprecated here; set it only on the strain "
               "calculator.");
}

void
ComputeLagrangianStressBase::initQpStatefulProperties()
{
  // Actually no need to zero out the stresses as they aren't stateful (yet)
}

void
ComputeLagrangianStressBase::computeQpProperties()
{
  computeQpStressUpdate();
  // Chain the kinematic policy into the PK1 Jacobian so the TL kernel consumes a single
  // d(PK1)/d(grad u) property and doesn't need to know about the generalized-alpha weighting.
  // For alpha = 1 (default) this is _pk1_jacobian itself. These are read only during Jacobian
  // assembly, so skip the R4*R4 work on residual-only sweeps.
  if (_fe_problem.currentlyComputingJacobian() ||
      _fe_problem.currentlyComputingResidualAndJacobian())
  {
    _dpk1_d_grad_u[_qp] = _pk1_jacobian[_qp] * _d_F_d_grad_u[_qp];

    // Non-local F-bar operator, composed once per qp here (shared by all displacement kernels)
    // instead of per-kernel in the assembly precalculate. `isPropertyActive`-gated so only the
    // F-bar (stabilized) kernels pay for the R4*R4*R4 chain.
    if (isPropertyActive(_d_nl_fbar.id()))
      _d_nl_fbar[_qp] = _cauchy_jacobian[_qp] * _d_deformation_gradient_increment_d_F[_qp] *
                        _d_F_stab_d_F_avg[_qp];
  }
}
