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
#include "RankFourTensorForward.h"
#include "RankTwoTensorForward.h"
#include "StabilizationUtils.h"
#include "GradientOperator.h"

/// Calculate strains to use the MOOSE materials with the Lagrangian kernels
///
/// This class calculates strain measures used by ComputeLagrangianStress
/// derived materials and used with
/// UpdatedLagrangianStressDivergence and TotalLagrangianStressDivergence
/// kernels
///
/// It has two basic jobs
/// 1) Calculate the deformation gradient at time steps n+1 and n
///    (the MOOSE material system doesn't bother for the SmallStrain case)
///    This includes including F_bar stabilization, if requested
/// 2) Calculate the kinematic quantities needed by the kernels:
///   a) The incremental inverse deformation gradient
///   b) The inverse deformation gradient
///   c) The determinant of the current deformation gradient
///
/// If required by the stabilize_strain flag it averages the pressure parts
/// of the deformation gradient.
///
/// This object cooperates with the homogenization constraint system by
/// adding in the scalar field representing the macroscale displacement
/// gradient before calculating strains.
///
template <class G>
class ComputeLagrangianStrainBase : public Material, public G
{
public:
  static InputParameters baseParams();
  static InputParameters validParams();
  ComputeLagrangianStrainBase(const InputParameters & parameters);
  virtual void initialSetup() override;

  /// Approximation used to convert the inverse incremental deformation gradient `f^{-1}`
  /// into the increment in the spatial velocity gradient (and its symmetric / skew parts).
  /// See `rashid_project/plan_outline.pdf` section 2.
  enum class KinematicApproximation
  {
    Linear,             ///< dL = I - f^{-1}
    Quadratic,          ///< dL = (I - f^{-1}) + 0.5 (I - f^{-1})^2
    RashidApproximate,  ///< Rashid's symmetric+skew formulas
    RashidEigen         ///< "Exact": polar decomposition + matrix logs
  };

  /// What F gets F-bar volumetric correction applied to. Affects only `stabilize_strain = true`.
  ///   Total      → average the full deformation gradient F_ust at each qp, then rescale by
  ///                gamma = cbrt(det(F_avg) / det(F_ust[qp])). This is the existing behavior.
  ///   Incremental → average the *incremental* F (`f_ust = F_ust · F_ust_old^{-1}`) at each qp,
  ///                 then rescale by gamma = cbrt(det(f_avg) / det(f_ust[qp])). Matches the
  ///                 OLD `ComputeFiniteStrain` F-bar (averaged Fhat) so cumulative strain is
  ///                 bit-for-bit compatible with `volumetric_locking_correction = true`.
  enum class FBarMode
  {
    Total,
    Incremental
  };

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeProperties() override;
  virtual void computeQpProperties() override;
  /// Calculate the strains based on the spatial velocity gradient. The sym/skew split is
  /// trivial; kept as a public helper for backward compatibility with the linear-only path.
  virtual void computeQpIncrementalStrains(const RankTwoTensor & dL);
  /// Update strain / vorticity / mechanical-strain bookkeeping from already-split
  /// (`dd`, `dw`) tensors. Used by the Rashid options where `dd != sym(dL)` and
  /// `dw != skew(dL)`.
  void setQpIncrementalStrains(const RankTwoTensor & dd, const RankTwoTensor & dw);
  /// Dispatcher: compute (Δd, Δw, d(Δl)/d(f^{-1}), d(Δw)/d(f^{-1})) for the active
  /// kinematic approximation. The vorticity derivative is returned separately so the
  /// Jaumann objective rate can chain its own Jacobian without re-projecting from dL.
  void computeQpLargeKinematicIncrement(const RankTwoTensor & f_inv,
                                        RankTwoTensor & dd,
                                        RankTwoTensor & dw,
                                        RankFourTensor & d_dL_d_f_inv,
                                        RankFourTensor & d_dw_d_f_inv);
  /// Compute and publish the polar decomposition of _F_actual at the current qp
  /// (R, U, dR/dF). Consumed by the Green-Naghdi objective rate.
  void computeQpPolarDecomposition();
  /// Subtract the eigenstrain increment to subtract from the total strain
  virtual void subtractQpEigenstrainIncrement(RankTwoTensor & strain);
  /// Calculate the unstabilized (alpha-weighted) deformation gradient at the quadrature point
  virtual void computeQpUnstabilizedDeformationGradient();
  /// Calculate the actual deformation gradient at n+1 (no alpha weighting, no F-bar)
  virtual void computeQpActualDeformationGradient();
  /// Calculate the unstabilized and optionally the stabilized deformation gradients
  virtual void computeDeformationGradient();

  // Displacements and displacement gradients
  const unsigned int _ndisp;
  std::vector<const VariableValue *> _disp;
  std::vector<const VariableGradient *> _grad_disp;
  /// Old displacement values for the generalized midpoint rule
  std::vector<const VariableValue *> _disp_old;
  /// Old displacement gradients for the generalized midpoint rule
  std::vector<const VariableGradient *> _grad_disp_old;

  /// Material system base name
  const std::string _base_name;

  /// If true the equilibrium conditions is calculated with large deformations
  const bool _large_kinematics;

  /// If true stabilize the strains with F_bar
  const bool _stabilize_strain;

  /// Selected F-bar averaging mode (Total vs. Incremental). See `FBarMode`.
  const FBarMode _F_bar_mode;

  /// Generalized-midpoint weight for the deformation gradient (1.0 = backward Euler, 0.5 = midpoint)
  const Real _alpha;

  /// Selected approximation for the spatial velocity gradient increment.
  const KinematicApproximation _kinematic_approximation;

  // The eigenstrains
  std::vector<MaterialPropertyName> _eigenstrain_names;
  std::vector<const MaterialProperty<RankTwoTensor> *> _eigenstrains;
  std::vector<const MaterialProperty<RankTwoTensor> *> _eigenstrains_old;

  // The total strains
  MaterialProperty<RankTwoTensor> & _total_strain;
  const MaterialProperty<RankTwoTensor> & _total_strain_old;
  MaterialProperty<RankTwoTensor> & _mechanical_strain;
  const MaterialProperty<RankTwoTensor> & _mechanical_strain_old;

  /// Mechanical strain accumulated with the incremental rotation r̂ = exp(Δw), matching
  /// the convention `ComputeFiniteStrain` uses for its `mechanical_strain` output
  /// (`ε_n+1 = r̂ · (ε_n + Δd) · r̂^T`). Provided as a separate property so the standard
  /// `_mechanical_strain` (consumed by constitutive materials) stays un-rotated and the
  /// objective-rate-driven stress chain is unaffected. Use this for aux-variable output
  /// when matching the old-system convention.
  MaterialProperty<RankTwoTensor> & _rotated_mechanical_strain;
  const MaterialProperty<RankTwoTensor> & _rotated_mechanical_strain_old;

  /// Strain increment
  MaterialProperty<RankTwoTensor> & _strain_increment;

  /// Deformation gradient increment
  MaterialProperty<RankTwoTensor> & _deformation_gradient_increment;

  /// Vorticity increment
  MaterialProperty<RankTwoTensor> & _vorticity_increment;

  /// The unstabilized (alpha-weighted) deformation gradient
  MaterialProperty<RankTwoTensor> & _F_ust;

  /// Old unstabilized deformation gradient. Needed for `F_bar_mode = incremental` so the
  /// incremental F (`F_ust · F_ust_old^{-1}`) at this step is built from the unstabilized
  /// pair (matching OLD `ComputeFiniteStrain`'s `_Fhat`). Only consulted in incremental mode.
  const MaterialProperty<RankTwoTensor> & _F_ust_old;

  /// The literal deformation gradient at n+1 (I + grad u_{n+1}). Equals _F_ust when alpha = 1.
  /// Needed by the UL kernel for the spatial-to-reference pull-back, since the spatial frame is
  /// always at n+1 regardless of alpha.
  MaterialProperty<RankTwoTensor> & _F_actual;

  // The average deformation gradient over the element for F-bar stabilization. Note that the
  // average deformation gradient is undefined if stabilization is not active.
  MaterialProperty<RankTwoTensor> & _F_avg;

  // The deformation gradient. If stabilization is active, this will be the stabilized deformation
  // gradient. Otherwise this will be equal to the unstabilized deformation gradient.
  MaterialProperty<RankTwoTensor> & _F;

  /// Old deformation gradient
  const MaterialProperty<RankTwoTensor> & _F_old;

  /// Inverse deformation gradient
  MaterialProperty<RankTwoTensor> & _F_inv;
  /// Inverse incremental deformation gradient
  MaterialProperty<RankTwoTensor> & _f_inv;

  /// Derivative of the spatial velocity gradient increment with respect to F_{n+1}.
  /// Stored so downstream consumers do not bake in the linear-approximation assumption.
  MaterialProperty<RankFourTensor> & _d_spatial_velocity_increment_d_F;

  /// Derivative of the vorticity increment Δw with respect to F_{n+1}. Consumed by the
  /// Jaumann objective rate so it can produce a consistent Jacobian regardless of which
  /// kinematic_approximation the strain calculator is using.
  MaterialProperty<RankFourTensor> & _d_vorticity_increment_d_F;

  /// Derivative of F_{n+1} with respect to the displacement gradient.
  /// Identity for backward Euler; will become alpha * Identity for the generalized midpoint rule.
  MaterialProperty<RankFourTensor> & _d_F_d_grad_u;

  /// Polar decomposition R of the (alpha-weighted, F-bar-stabilized) deformation gradient
  /// F at this qp. Stateful so the Green-Naghdi rate can read R_n via getMaterialPropertyOld.
  MaterialProperty<RankTwoTensor> & _rotation;
  const MaterialProperty<RankTwoTensor> & _rotation_old;
  /// Stretch U from the same polar decomposition.
  MaterialProperty<RankTwoTensor> & _stretch;
  /// Derivative of R with respect to F.
  MaterialProperty<RankFourTensor> & _d_rotation_d_F;

  /// Partial derivative of the F-bar-stabilized deformation gradient with respect to the
  /// unstabilized (per-qp local) deformation gradient. Equals IdentityFour when F-bar is off.
  MaterialProperty<RankFourTensor> & _d_F_stab_d_F_ust;

  /// Partial derivative of the F-bar-stabilized deformation gradient with respect to the
  /// element-averaged deformation gradient F_avg (the non-local coupling). Equals zero when
  /// F-bar is off.
  MaterialProperty<RankFourTensor> & _d_F_stab_d_F_avg;

  /// Names of any extra homogenization gradients
  std::vector<MaterialPropertyName> _homogenization_gradient_names;

  /// Actual homogenization contributions
  std::vector<const MaterialProperty<RankTwoTensor> *> _homogenization_contributions;

  /// Rotation increment for "old" materials inheriting from ComputeStressBase
  MaterialProperty<RankTwoTensor> & _rotation_increment;

private:
  /// Linear approximation: dL = I - f^{-1}.
  void computeLinearIncrement(const RankTwoTensor & f_inv,
                              RankTwoTensor & dd,
                              RankTwoTensor & dw,
                              RankFourTensor & d_dL_d_f_inv,
                              RankFourTensor & d_dw_d_f_inv) const;
  /// Quadratic approximation: dL = (I - f^{-1}) + 0.5 (I - f^{-1})^2.
  void computeQuadraticIncrement(const RankTwoTensor & f_inv,
                                 RankTwoTensor & dd,
                                 RankTwoTensor & dw,
                                 RankFourTensor & d_dL_d_f_inv,
                                 RankFourTensor & d_dw_d_f_inv) const;
  /// Rashid's approximate symmetric+skew formulas.
  void computeRashidApproximateIncrement(const RankTwoTensor & f_inv,
                                         RankTwoTensor & dd,
                                         RankTwoTensor & dw,
                                         RankFourTensor & d_dL_d_f_inv,
                                         RankFourTensor & d_dw_d_f_inv) const;
  /// "Exact" via polar decomposition of f^{-1} + matrix logs.
  void computeRashidEigenIncrement(const RankTwoTensor & f_inv,
                                   RankTwoTensor & dd,
                                   RankTwoTensor & dw,
                                   RankFourTensor & d_dL_d_f_inv,
                                   RankFourTensor & d_dw_d_f_inv) const;
};
