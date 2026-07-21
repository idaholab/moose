//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "RankTwoTensorForward.h"
#include "RankFourTensorForward.h"

/// Provide stresses in the form required for the Lagrangian kernels
///
/// This base class represents a material interface "contract"
///
/// As input it takes the deformation measures provided by
/// ComputeLagrangianStrain:
///   1) "deformation_gradient" -- the deformation gradient
///   2) "inv_inc_def_grad" -- the inverse incremental deformation gradient
///   3) "inv_def_grad" -- the inverse deformation gradient
///   4) "mechanical_strain" -- the integrated deformation rate (log strain)
///   5) "strain_inc" -- the increment in the deformation rate
///
/// Additional strain measures could be defined by subclassing
/// ComputeLagrangianStrain
///
/// And return:
///   1) "stress" -- the cauchy stress
///   2) "cauchy_jacobian" -- the derivative of the increment in the
///                           cauchy stress with respect to the increment
///                           in the spatial velocity gradient
///   3) "pk1_stress" -- the first Piola-Kirchhoff stress
///   4) "pk1_jacobian" -- the derivative of the first PK stress with respect
///                           to the deformation gradient
///
/// Both stress measures and derivatives are required to interface
/// with both the total and updated Lagrangian kernels.
///
/// The class takes the "large_kinematics" flag as input
///
/// If true then the above stress and strain measures will be different
/// and the class is required to use correct large deformation kinematics
///
/// If false all the stress measures and all the strain measures except
/// the deformation gradient are equivalent and the stress update can use
/// small deformation kinematics.  The deformation gradient is the identity
/// for small deformation kinematics, as we use it as a push-forward in the
/// kernel.
///
class ComputeLagrangianStressBase : public Material
{
public:
  static InputParameters validParams();
  ComputeLagrangianStressBase(const InputParameters & parameters);

protected:
  /// Initialize everything with zeros
  virtual void initQpStatefulProperties() override;
  /// Update all properties (here just the stress/derivatives)
  virtual void computeQpProperties() override;
  /// Provide for the actual stress updates
  virtual void computeQpStressUpdate() = 0;

protected:
  /// If true use large deformations
  const bool _large_kinematics;

  /// Prepend to the material properties
  const std::string _base_name;

  /// The Cauchy stress
  MaterialProperty<RankTwoTensor> & _cauchy_stress;
  /// The derivative of the Cauchy stress wrt the increment in the spatial
  /// velocity gradient
  MaterialProperty<RankFourTensor> & _cauchy_jacobian;

  /// The 1st Piola-Kirchhoff stress
  MaterialProperty<RankTwoTensor> & _pk1_stress;
  /// The derivative of the 1st PK stress wrt the deformation gradient (F that the stress
  /// material consumes; with the generalized midpoint rule this is the alpha-weighted F).
  MaterialProperty<RankFourTensor> & _pk1_jacobian;
  /// Variant of `_pk1_jacobian` computed WITHOUT the F-bar chain factor
  /// `_d_F_stab_d_F_ust` in the sigma-via-dL contribution. Used by specialty kernels
  /// (weak plane stress, homogenization macro_grad) whose coupled variable adds to
  /// `_F` AFTER F-bar has run -- those perturbations bypass F-bar's chain and need the
  /// identity F-bar partial in the sigma chain to give a consistent Jacobian.
  MaterialProperty<RankFourTensor> & _pk1_jacobian_bypass_fbar;

  /// The derivative of the 1st PK stress wrt the displacement gradient (grad u_{n+1}).
  /// Computed in computeQpProperties as _pk1_jacobian * _d_F_d_grad_u so the TL kernel can
  /// consume a single property without needing to know the generalized-alpha kinematic
  /// policy. For alpha = 1 (default) it equals _pk1_jacobian.
  MaterialProperty<RankFourTensor> & _dpk1_d_grad_u;

  /// d(F)/d(grad u_{n+1}) (= alpha * IdentityFour for the generalized midpoint rule)
  const MaterialProperty<RankFourTensor> & _d_F_d_grad_u;

  /// Unstabilized deformation gradient (= F_actual at alpha = 1). Used by the stress
  /// materials as F in the kinematic stress-measure wraps (sigma <-> PK1 <-> PK2) so that the
  /// residual matches OLD's `StressDivergenceTensors`-on-displaced-mesh form. F-bar
  /// enters only through the constitutive stress via the strain calc's F-bar'd `_f_inv`.
  const MaterialProperty<RankTwoTensor> & _F_ust;

  /// d(spatial velocity gradient increment)/d(F_stab). Used in the sigma-via-dL chain to
  /// build pk1_jacobian / cauchy_jacobian.
  const MaterialProperty<RankFourTensor> & _d_deformation_gradient_increment_d_F;

  /// d(F_stab)/d(F_ust). Multiplied into the sigma chain so pk1_jacobian = dPK1/d(F_ust)
  /// (and cauchy_jacobian = dsigma/d(dL)) gets the local F-bar contribution. Equals
  /// IdentityFour when F-bar is off.
  const MaterialProperty<RankFourTensor> & _d_F_stab_d_F_ust;

  /// d(F_stab)/d(F_avg), the non-local F-bar partial. Read to compose `_d_nl_fbar`.
  const MaterialProperty<RankFourTensor> & _d_F_stab_d_F_avg;

  /// Non-local F-bar operator D_nl = cauchy_jacobian : d(dL)/dF : d(F_stab)/d(F_avg), published
  /// so the Lagrangian kernels' non-local F-bar Jacobian term reads a per-qp material property
  /// (computed once here, shared by all displacement kernels) instead of rebuilding this R4*R4*R4
  /// chain per-kernel. Jacobian-only and `isPropertyActive`-gated (only F-bar kernels mark it
  /// active); left untouched on residual-only sweeps and non-stabilized runs.
  MaterialProperty<RankFourTensor> & _d_nl_fbar;
};
