//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeLagrangianStressCauchy.h"
#include "LagrangianObjectiveRate.h"

/// Provide the Cauchy stress via an objective integration of a small stress
///
/// This class provides the Cauchy stress and associated Jacobian through
/// an objective integration of the small strain constitutive model.
///
/// The small strain model implements the computeQpSmallStress()
/// which must provide the _small_stress and _small_jacobian
/// properties.
///
/// The objective integration itself is delegated to a stateless free function selected by the
/// `objective_rate` input parameter (truesdell / jaumann / green_naghdi / rashid). This material
/// gathers the kinematic / constitutive quantities into a `LagrangianObjectiveRates::Inputs`, calls
/// `LagrangianObjectiveRates::compute`, and scatters the returned Cauchy stress, consistent
/// Jacobian, and eigenstrain off-diagonal Jacobian back into its properties.
class ComputeLagrangianObjectiveStress : public ComputeLagrangianStressCauchy
{
public:
  static InputParameters validParams();
  ComputeLagrangianObjectiveStress(const InputParameters & parameters);

protected:
  /// Initialize the new (small) stress
  virtual void initQpStatefulProperties() override;

  /// Implement the objective update
  virtual void computeQpCauchyStress() override;

  /// Method to implement to provide the small stress update
  //    This method must provide the _small_stress and _small_jacobian
  virtual void computeQpSmallStress() = 0;

  /// The updated small stress
  MaterialProperty<RankTwoTensor> & _small_stress;

  /// We need the old value to get the increment
  const MaterialProperty<RankTwoTensor> & _small_stress_old;

  /// The updated small algorithmic tangent
  MaterialProperty<RankFourTensor> & _small_jacobian;

  /// Derivative of the Cauchy stress with respect to the eigenstrain at this step,
  /// d_sigma/d_eigenstrain = -Jinv_obj * small_jacobian. The kernel consumes this single
  /// rank-4 property to assemble the temperature off-diagonal Jacobian; future Cauchy-
  /// providing materials that don't go through the objective-rate path can publish this
  /// property with their own derivative.
  MaterialProperty<RankFourTensor> & _dcauchy_stress_d_eigenstrain;

  /// We need the old Cauchy stress to do the objective integration
  const MaterialProperty<RankTwoTensor> & _cauchy_stress_old;

  /// Provided for material models that use the integrated strain
  const MaterialProperty<RankTwoTensor> & _mechanical_strain;

  /// Provided for material models that use the strain increment
  const MaterialProperty<RankTwoTensor> & _strain_increment;

  /// Provided for material models that use the vorticity increment
  const MaterialProperty<RankTwoTensor> & _vorticity_increment;

  /// The spatial velocity gradient increment (dL)
  const MaterialProperty<RankTwoTensor> & _deformation_gradient_increment;

  /// d(dL)/dF, stored by the strain calculator
  const MaterialProperty<RankFourTensor> & _d_deformation_gradient_increment_d_F;

  /// d(dW)/dF from the strain calculator, consumed by Jaumann and Rashid
  const MaterialProperty<RankFourTensor> & _d_vorticity_increment_d_F;

  /// Polar-decomposition rotation R of F (and its old value and derivative), published by the
  /// strain calculator and consumed only by the Green-Naghdi rate. Fetched (and thereby marked
  /// active, which is what triggers the strain calc's polar decomposition) only when
  /// `objective_rate = green_naghdi`; nullptr for the other rates.
  const MaterialProperty<RankTwoTensor> * _rotation = nullptr;
  const MaterialProperty<RankTwoTensor> * _rotation_old = nullptr;
  const MaterialProperty<RankFourTensor> * _d_rotation_d_F = nullptr;

  /// Deformation gradient
  const MaterialProperty<RankTwoTensor> & _def_grad;

  /// Deformation gradient
  const MaterialProperty<RankTwoTensor> & _def_grad_old;

  /// Which objective rate to use (truesdell / jaumann / green_naghdi / rashid). Dispatched by
  /// `LagrangianObjectiveRates::compute` at each qp.
  const MooseEnum & _objective_rate;

  /// If true, the rate runs in passthrough mode -- the host discards the rate's own outer
  /// rotation and sets `_cauchy_stress = _small_stress` directly. Used in tandem with the
  /// strain calculator's `publish_rotation_increment = true` to delegate the rotation of
  /// the stress state to the wrapped material's `_perform_finite_strain_rotations = true`
  /// path (so `ComputeMultiPlasticityStress` etc. see the correctly-rotated `_stress_old`
  /// for return mapping). `_cauchy_jacobian` is still built by the rate's chain rule, so
  /// the Jacobian remains consistent.
  const bool _rotate_old_stress;
};
